extends Control

@onready var sudoku: SudokuNode = $SudokuNode
@onready var grid: GridContainer = $CenterContainer/GameLayout/GridContainer
@onready var difficulty: OptionButton = $CenterContainer/GameLayout/Toolbar/Difficulty
@onready var variant: OptionButton = $CenterContainer/GameLayout/Toolbar/Variant
@onready var title_label: Label = $CenterContainer/GameLayout/Title
@onready var feedback: AcceptDialog = $Feedback
@onready var hint_button: Button = $CenterContainer/GameLayout/Toolbar/Hint
@onready var solve_button: Button = $CenterContainer/GameLayout/Toolbar/Solve
@onready var lives_label: Label = $CenterContainer/GameLayout/Toolbar/Lives
@onready var number_pad: HBoxContainer = $CenterContainer/GameLayout/NumberPad
@onready var undo_button: Button = $CenterContainer/GameLayout/Tools/Undo
@onready var redo_button: Button = $CenterContainer/GameLayout/Tools/Redo
@onready var draft_button: Button = $CenterContainer/GameLayout/Tools/Draft
@onready var csp_demo: Node = $CspDemo

const DIFFICULTIES := ["easy", "medium", "hard"]
const VARIANTS := ["classic", "jigsaw"]
# jjigsaw
const REGION_COLORS := [Color("29364a"), Color("303e40"), Color("3b3447"),
	Color("3d3a31"), Color("273e46"), Color("3c333c"),
	Color("343d32"), Color("31324a"), Color("3f3734")]
const COLOR_CELL := Color("253041")
const COLOR_MATCH := Color("304159")
const COLOR_SELECTED := Color("345478")
const COLOR_HINT := Color("304e46")
const COLOR_HINT_SELECTED := Color("3c665a")
const COLOR_WRONG := Color("683b43")
const COLOR_WRONG_TEXT := Color("ff8e99")
const WRONG_FLASH_SECONDS := 1.2
const STARTING_LIVES := 3

var cells: Array[Button] = []
var note_layers: Array[Control] = []
var selected_row: int = -1
var selected_col: int = -1
var chosen_number: int = 0
var active_number: int = 0
var eraser_active: bool = false
var hinted_cell: int = -1
var drafting: bool = false
var drafts: Array[int] = []
var lives: int = STARTING_LIVES
var wrong_cells: Array[int] = []
var checking_board: bool = false
var undo_stack: Array[Dictionary] = []
var redo_stack: Array[Dictionary] = []
var puzzle_version: int = 0
var regions: Array = []
var is_jigsaw: bool = false

func _ready() -> void:
	drafts.resize(81)
	drafts.fill(0)
	_build_grid()
	_build_number_pad()
	start_game("easy")

func _can_edit() -> bool:
	return sudoku.has_puzzle() and lives > 0 and not checking_board and not csp_demo.running

func _build_number_pad() -> void:
	for value in range(1, 10):
		var button := Button.new()
		button.text = str(value)
		button.custom_minimum_size = Vector2(40, 44)
		button.size_flags_horizontal = Control.SIZE_EXPAND_FILL
		button.focus_mode = Control.FOCUS_NONE
		button.toggle_mode = true
		button.add_theme_font_size_override("font_size", 20)
		button.pressed.connect(_choose_number.bind(value))
		number_pad.add_child(button)
	var erase := Button.new()
	erase.text = "Erase"
	erase.custom_minimum_size = Vector2(60, 44)
	erase.focus_mode = Control.FOCUS_NONE
	erase.toggle_mode = true
	erase.pressed.connect(_choose_eraser)
	number_pad.add_child(erase)

func _build_grid() -> void:
	for i in range(81):
		var btn := Button.new()
		btn.custom_minimum_size = Vector2(52, 52)
		btn.focus_mode = Control.FOCUS_NONE
		btn.add_theme_font_size_override("font_size", 24)
		btn.pressed.connect(_on_cell_pressed.bind(i))
		grid.add_child(btn)
		cells.append(btn)
		var notes := Control.new()
		notes.mouse_filter = Control.MOUSE_FILTER_IGNORE
		notes.set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT)
		notes.offset_left = 3
		notes.offset_top = 1
		notes.offset_right = -3
		notes.offset_bottom = -1
		for digit in range(1, 10):
			var label := Label.new()
			label.mouse_filter = Control.MOUSE_FILTER_IGNORE
			var slot := digit - 1
			var slot_row: int = slot / 3
			var slot_col: int = slot % 3
			label.anchor_left = slot_col / 3.0
			label.anchor_top = slot_row / 3.0
			label.anchor_right = (slot_col + 1) / 3.0
			label.anchor_bottom = (slot_row + 1) / 3.0
			label.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
			label.vertical_alignment = VERTICAL_ALIGNMENT_CENTER
			label.size_flags_horizontal = Control.SIZE_EXPAND_FILL
			label.size_flags_vertical = Control.SIZE_EXPAND_FILL
			label.add_theme_font_size_override("font_size", 10)
			label.add_theme_color_override("font_color", Color("a9bbcf"))
			notes.add_child(label)
		btn.add_child(notes)
		note_layers.append(notes)

func _cell_style(row: int, col: int, selected: bool, matching: bool, hinted: bool) -> StyleBoxFlat:
	var style := StyleBoxFlat.new()
	var cell := row * 9 + col
	var base_color: Color = REGION_COLORS[regions[cell]] if is_jigsaw else COLOR_CELL
	style.bg_color = COLOR_MATCH if matching else base_color
	if selected:
		style.bg_color = COLOR_SELECTED
	if hinted:
		style.bg_color = COLOR_HINT_SELECTED if selected else COLOR_HINT
	if row * 9 + col in wrong_cells:
		style.bg_color = COLOR_WRONG
	style.border_color = Color("718096")
	style.border_width_left = 2 if col == 0 or regions[cell] != regions[cell - 1] else 0
	style.border_width_top = 2 if row == 0 or regions[cell] != regions[cell - 9] else 0
	style.border_width_right = 2 if col == 8 else 1
	style.border_width_bottom = 2 if row == 8 else 1
	return style

func _refresh() -> void:
	var board: Array = sudoku.get_board()
	var clues: Array = sudoku.get_clues()
	regions = sudoku.get_regions()
	is_jigsaw = sudoku.get_variant() == "jigsaw"
	var editable := _can_edit()
	for i in range(81):
		var value: int = board[i]
		var btn := cells[i]
		var row: int = i / 9
		var col: int = i % 9
		var selected := row == selected_row and col == selected_col
		btn.text = "" if value == 0 else str(value)
		btn.disabled = not editable
		var style := _cell_style(row, col, selected, chosen_number > 0 and value == chosen_number, i == hinted_cell)
		for state in ["normal", "hover", "pressed", "disabled"]:
			btn.add_theme_stylebox_override(state, style)
		var color := Color("edf2f7") if clues[i] else Color("74c5ff")
		if i in wrong_cells:
			color = COLOR_WRONG_TEXT
		for state in ["font_color", "font_hover_color", "font_pressed_color", "font_disabled_color"]:
			btn.add_theme_color_override(state, color)
		note_layers[i].visible = value == 0
		for digit in range(1, 10):
			var label := note_layers[i].get_child(digit - 1) as Label
			label.text = str(digit) if drafts[i] & (1 << (digit - 1)) else ""
	hint_button.disabled = not editable or sudoku.is_solved()
	solve_button.disabled = hint_button.disabled
	lives_label.text = "%d/3" % lives
	lives_label.modulate = COLOR_WRONG_TEXT if lives == 0 else Color.WHITE
	var game_title := "Jigsaw Sudoku" if is_jigsaw else "Sudoku"
	if lives == 0:
		title_label.text = game_title + " - Game Over"
	elif sudoku.is_solved():
		title_label.text = game_title + " - solved!"
	else:
		title_label.text = game_title
	for i in range(9):
		var button := number_pad.get_child(i) as Button
		button.disabled = not editable
		button.set_pressed_no_signal(active_number == i + 1)
	var erase := number_pad.get_child(9) as Button
	erase.disabled = not editable
	erase.set_pressed_no_signal(eraser_active)
	draft_button.disabled = not editable
	draft_button.set_pressed_no_signal(drafting)
	undo_button.disabled = not editable or undo_stack.is_empty()
	redo_button.disabled = not editable or redo_stack.is_empty()
	csp_demo.refresh()

func _on_cell_pressed(index: int) -> void:
	if not _can_edit():
		return
	selected_row = index / 9
	selected_col = index % 9
	var value: int = sudoku.get_board()[index]
	if eraser_active:
		_erase_cell()
		return
	if active_number > 0:
		_enter_number(active_number)
		return
	if value > 0:
		chosen_number = value
	_refresh()

func _choose_number(value: int) -> void:
	if not _can_edit():
		return
	active_number = value
	eraser_active = false
	chosen_number = value
	_refresh()

func _choose_eraser() -> void:
	if not _can_edit():
		return
	active_number = 0
	eraser_active = true
	chosen_number = 0
	_refresh()

func _on_draft_toggled(enabled: bool) -> void:
	drafting = enabled
	_refresh()

func _snapshot() -> Dictionary:
	return {"board": sudoku.get_board(), "drafts": drafts.duplicate(), "hinted": hinted_cell}

func _record_action(before: Dictionary) -> void:
	var after := _snapshot()
	if before == after:
		return
	undo_stack.append({"before": before, "after": after})
	redo_stack.clear()

func _restore_snapshot(state: Dictionary) -> void:
	var board: Array = state["board"]
	var clues: Array = sudoku.get_clues()
	for i in range(81):
		if clues[i]:
			continue
		var row: int = i / 9
		var col: int = i % 9
		if board[i] == 0:
			sudoku.clear_cell(row, col)
		else:
			sudoku.set_cell(row, col, board[i])
	drafts.assign(state["drafts"])
	hinted_cell = state["hinted"]

func undo() -> void:
	if not _can_edit() or undo_stack.is_empty():
		return
	var action: Dictionary = undo_stack.pop_back()
	_restore_snapshot(action["before"])
	redo_stack.append(action)
	_refresh()

func redo() -> void:
	if not _can_edit() or redo_stack.is_empty():
		return
	var action: Dictionary = redo_stack.pop_back()
	_restore_snapshot(action["after"])
	undo_stack.append(action)
	_refresh()

func _finish_action(before: Dictionary) -> void:
	if sudoku.count_empty() == 0 and not sudoku.is_solved():
		checking_board = true
		lives -= 1
		var board: Array = sudoku.get_board()
		var solution: Array = sudoku.get_solution()
		for i in range(81):
			if board[i] != solution[i]:
				wrong_cells.append(i)
		_refresh()
		var version := puzzle_version
		await get_tree().create_timer(WRONG_FLASH_SECONDS).timeout
		if version != puzzle_version:
			return
		for i in wrong_cells:
			var row: int = i / 9
			var col: int = i % 9
			sudoku.clear_cell(row, col)
			drafts[i] = 0
			if hinted_cell == i:
				hinted_cell = -1
		wrong_cells.clear()
		checking_board = false
	_record_action(before)
	_refresh()

func _enter_number(value: int) -> void:
	if not _can_edit():
		return
	chosen_number = value
	if selected_row == -1:
		_refresh()
		return
	var index := selected_row * 9 + selected_col
	var clues: Array = sudoku.get_clues()
	if clues[index]:
		_refresh()
		return
	var before := _snapshot()
	if drafting:
		if sudoku.get_board()[index] == 0:
			drafts[index] ^= 1 << (value - 1)
	else:
		if sudoku.set_cell(selected_row, selected_col, value):
			drafts[index] = 0
			if hinted_cell == index:
				hinted_cell = -1
	_finish_action(before)

func _erase_cell() -> void:
	if not _can_edit() or selected_row == -1:
		return
	var index := selected_row * 9 + selected_col
	if sudoku.get_clues()[index]:
		return
	var before := _snapshot()
	if sudoku.clear_cell(selected_row, selected_col):
		drafts[index] = 0
		if hinted_cell == index:
			hinted_cell = -1
		chosen_number = active_number
	_finish_action(before)

func _unhandled_key_input(event: InputEvent) -> void:
	if not event is InputEventKey or not _can_edit():
		return
	var key_event := event as InputEventKey
	if not key_event.pressed or key_event.echo:
		return
	if key_event.ctrl_pressed:
		if key_event.keycode == KEY_Z:
			if key_event.shift_pressed:
				redo()
			else:
				undo()
		elif key_event.keycode == KEY_Y:
			redo()
		else:
			return
		get_viewport().set_input_as_handled()
		return
	if selected_row == -1:
		return
	var key := key_event.keycode
	var value := 0
	if key >= KEY_1 and key <= KEY_9:
		value = int(key) - int(KEY_0)
	elif key >= KEY_KP_1 and key <= KEY_KP_9:
		value = int(key) - int(KEY_KP_0)
	elif key == KEY_DELETE or key == KEY_BACKSPACE:
		_erase_cell()
		get_viewport().set_input_as_handled()
		return
	else:
		return
	_choose_number(value)
	_enter_number(value)
	get_viewport().set_input_as_handled()

func start_game(level: String) -> void:
	csp_demo.stop()
	if not sudoku.new_game(level, VARIANTS[variant.selected]):
		_show_message("Couldn't generate a puzzle. Try again.")
		return
	puzzle_version += 1
	lives = STARTING_LIVES
	checking_board = false
	wrong_cells.clear()
	drafts.fill(0)
	drafting = false
	undo_stack.clear()
	redo_stack.clear()
	feedback.hide()
	_reset_selection()

func _reset_selection() -> void:
	selected_row = -1
	selected_col = -1
	chosen_number = 0
	active_number = 0
	eraser_active = false
	hinted_cell = -1
	_refresh()

func _on_generate_pressed() -> void:
	start_game(DIFFICULTIES[difficulty.selected])

func _show_message(message: String) -> void:
	feedback.dialog_text = message
	feedback.popup_centered()

func solve_all() -> void:
	if not _can_edit():
		return
	var before := _snapshot()
	if sudoku.solve():
		drafts.fill(0)
		hinted_cell = -1
		_record_action(before)
		_reset_selection()
	else:
		_show_message("No saved solution is available. Generate a puzzle first.")

func hint() -> void:
	if not _can_edit():
		return
	var before := _snapshot()
	if sudoku.give_hint():
		var after: Array = sudoku.get_board()
		for i in range(81):
			if before["board"][i] != after[i]:
				hinted_cell = i
				drafts[i] = 0
				break
		_finish_action(before)
