extends RefCounted
const ALL_DIGITS := 511
var board: Array[int] = []
var domains: Array[int] = []
var frames: Array[Dictionary] = []
var phase: String = "initial"
var finished: bool = false
var solved: bool = false
var steps: int = 0
var conflict_cell: int = -1

func begin(puzzle: Array) -> bool:
	board.clear()
	domains.clear()
	frames.clear()
	phase = "initial"
	finished = false
	solved = false
	steps = 0
	conflict_cell = -1
	if puzzle.size() != 81:
		return false
	for value in puzzle:
		if not value is int or value < 0 or value > 9:
			return false
		board.append(value)
	domains.resize(81)
	for cell in range(81):
		if board[cell] > 0:
			for peer in peers(cell):
				if board[peer] == board[cell]:
					return false
			domains[cell] = 1 << (board[cell] - 1)
		else:
			var mask := ALL_DIGITS
			for peer in peers(cell):
				if board[peer] > 0:
					mask &= ~(1 << (board[peer] - 1))
			domains[cell] = mask
	return true

static func peers(cell: int) -> Array[int]:
	var result: Array[int] = []
	var row: int = cell / 9
	var col: int = cell % 9
	for other in range(81):
		if other == cell:
			continue
		var other_row: int = other / 9
		var other_col: int = other % 9
		if row == other_row or col == other_col or (row / 3 == other_row / 3 and col / 3 == other_col / 3):
			result.append(other)
	return result

static func candidates(mask: int) -> Array[int]:
	var result: Array[int] = []
	for digit in range(1, 10):
		if mask & (1 << (digit - 1)):
			result.append(digit)
	return result

static func address(cell: int) -> String:
	var row: int = cell / 9
	return "r%d c%d" % [row + 1, cell % 9 + 1]

func _event(kind: String, cell: int, message: String, affected: Array[int] = []) -> Dictionary:
	return {"kind": kind, "cell": cell, "message": message, "affected": affected}

func advance() -> Dictionary:
	if finished:
		return _event("solved" if solved else "failed", -1, "Finished.")
	steps += 1
	match phase:
		"initial":
			phase = "select"
			return _event("domains", -1, "CSP: cells are variables; small numbers are their domains. Row, column and box constraints remove candidates. Starting from original clues.")
		"select":
			var best := -1
			var smallest := 10
			for cell in range(81):
				if board[cell] != 0:
					continue
				var count := candidates(domains[cell]).size()
				if count < smallest:
					best = cell
					smallest = count
			if best == -1:
				finished = true
				solved = true
				return _event("solved", -1, "Solved: every cell is assigned and all constraints are satisfied.")
			if smallest == 0:
				phase = "backtrack"
				return _event("conflict", best, "Contradiction: %s has no allowed value. Backtrack to the previous choice." % address(best))
			var choices := candidates(domains[best])
			frames.append({"cell": best, "remaining": choices, "board": board.duplicate(), "domains": domains.duplicate()})
			phase = "try"
			return _event("select", best, "MRV: choose %s, with the fewest candidates: %s." % [address(best), str(choices)])
		"try":
			var frame: Dictionary = frames.back()
			var cell: int = frame["cell"]
			var remaining: Array = frame["remaining"]
			if remaining.is_empty():
				phase = "backtrack"
				return _event("conflict", cell, "No untried candidate remains at %s. Backtrack." % address(cell))
			board.assign(frame["board"])
			domains.assign(frame["domains"])
			var digit: int = remaining.pop_front()
			board[cell] = digit
			domains[cell] = 1 << (digit - 1)
			phase = "propagate"
			var reason := "the only candidate" if candidates(frame["domains"][cell]).size() == 1 else "a candidate to test"
			return _event("assign", cell, "Assign %d at %s: %s. Next, propagate its constraints." % [digit, address(cell), reason])
		"propagate":
			var cell: int = frames.back()["cell"]
			var digit := board[cell]
			var affected: Array[int] = []
			conflict_cell = -1
			for peer in peers(cell):
				if board[peer] == 0 and domains[peer] & (1 << (digit - 1)):
					domains[peer] &= ~(1 << (digit - 1))
					affected.append(peer)
					if domains[peer] == 0 and conflict_cell == -1:
						conflict_cell = peer
			phase = "conflict" if conflict_cell >= 0 else "select"
			return _event("propagate", cell, "Forward checking: remove %d from %d empty peers in the same row, column or box." % [digit, affected.size()], affected)
		"conflict":
			phase = "backtrack"
			return _event("conflict", conflict_cell, "Contradiction: %s has an empty domain. This branch cannot work." % address(conflict_cell))
		"backtrack":
			if frames.is_empty():
				finished = true
				return _event("failed", -1, "No solution: every possible branch has been exhausted.")
			var frame: Dictionary = frames.back()
			var cell: int = frame["cell"]
			board.assign(frame["board"])
			domains.assign(frame["domains"])
			var retry: bool = not frame["remaining"].is_empty()
			if retry:
				phase = "try"
			else:
				frames.pop_back()
				phase = "backtrack"
			var next := "try its next candidate" if retry else "return to an earlier choice"
			return _event("backtrack", cell, "Backtrack: undo %s and restore its previous domains; %s." % [address(cell), next])
	return _event("failed", -1, "Unknown solver phase.")
