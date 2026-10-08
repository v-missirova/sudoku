extends Node

const Solver = preload("res://csp_demo_solver.gd")
const COLOR_FOCUS := Color("67582e")
const COLOR_AFFECTED := Color("2e485c")
const COLOR_CONFLICT := Color("683b43")

var host: Control
var solver = null
var running: bool = false
var paused: bool = false
var interval: float = 0.6
var elapsed: float = 0.0
var before: Dictionary = {}
var event: Dictionary = {}
var demo_button: Button
var panel: VBoxContainer
var explanation: Label
var pause_button: Button
var step_button: Button
var layout: VBoxContainer
var normal_separation: int

func _ready() -> void:
	host = get_parent()
	layout = host.get_node("CenterContainer/GameLayout") as VBoxContainer
	normal_separation = layout.get_theme_constant("separation")
	var toolbar := layout.get_node("Toolbar") as HBoxContainer
	demo_button = Button.new()
	demo_button.text = "CSP Solve"
	demo_button.focus_mode = Control.FOCUS_NONE
	demo_button.tooltip_text = "Show candidates, MRV, forward checking and backtracking from the original clues."
	demo_button.pressed.connect(toggle)
	toolbar.add_child(demo_button)
	panel = VBoxContainer.new()
	panel.name = "CspPanel"
	panel.add_theme_constant_override("separation", 4)
	panel.visible = false
	layout.add_child(panel)
	explanation = Label.new()
	explanation.custom_minimum_size = Vector2(468, 48)
	explanation.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
	explanation.add_theme_font_size_override("font_size", 13)
	panel.add_child(explanation)
	var controls := HBoxContainer.new()
	controls.alignment = BoxContainer.ALIGNMENT_CENTER
	controls.add_theme_constant_override("separation", 8)
	panel.add_child(controls)
	pause_button = Button.new()
	pause_button.text = "Pause"
	pause_button.focus_mode = Control.FOCUS_NONE
	pause_button.pressed.connect(toggle_pause)
	controls.add_child(pause_button)
	step_button = Button.new()
	step_button.text = "Step"
	step_button.focus_mode = Control.FOCUS_NONE
	step_button.pressed.connect(step_once)
	controls.add_child(step_button)
	var speed := OptionButton.new()
	speed.focus_mode = Control.FOCUS_NONE
	speed.add_item("Slow")
	speed.add_item("Normal")
	speed.add_item("Fast")
	speed.select(1)
	speed.item_selected.connect(_set_speed)
	controls.add_child(speed)

func _set_speed(index: int) -> void:
	interval = [1.2, 0.6, 0.08][index]
	elapsed = 0.0

func toggle() -> void:
	if running:
		stop()
	else:
		start()

func start() -> void:
	if running or not host._can_edit() or host.sudoku.get_variant() != "classic":
		return
	solver = Solver.new()
	if not solver.begin(host.sudoku.get_puzzle()):
		host._show_message("The original puzzle cannot be used for this demo.")
		return
	before = host._snapshot()
	running = true
	paused = false
	elapsed = 0.0
	panel.show()
	layout.add_theme_constant_override("separation", 8)
	event = solver.advance()
	host._refresh()

func stop() -> void:
	if not running:
		return
	running = false
	paused = false
	solver = null
	event.clear()
	panel.hide()
	layout.add_theme_constant_override("separation", normal_separation)
	before = {}
	host._refresh()

func toggle_pause() -> void:
	if not running:
		return
	paused = not paused
	elapsed = 0.0
	host._refresh()

func step_once() -> void:
	if not running:
		return
	paused = true
	elapsed = 0.0
	_advance()

func _process(delta: float) -> void:
	if not running or paused:
		return
	elapsed += delta
	if elapsed >= interval:
		elapsed = 0.0
		_advance()

func _advance() -> void:
	event = solver.advance()
	if solver.finished:
		if solver.solved:
			var clues: Array = host.sudoku.get_clues()
			for i in range(81):
				if not clues[i]:
					var row: int = i / 9
					var col: int = i % 9
					host.sudoku.set_cell(row, col, solver.board[i])
			host.drafts.fill(0)
			host.hinted_cell = -1
			host._record_action(before)
			host._reset_selection()
		else:
			host._show_message("CSP demonstration finished without a solution.")
		stop()
	else:
		host._refresh()

func refresh() -> void:
	demo_button.text = "Stop CSP" if running else "CSP Solve"
	demo_button.visible = host.sudoku.get_variant() == "classic"
	demo_button.disabled = not running and not host._can_edit()
	if not running:
		return
	pause_button.text = "Resume" if paused else "Pause"
	step_button.disabled = false
	explanation.text = "Step %d - %s" % [solver.steps, event["message"]]
	var focused: int = event["cell"]
	var affected: Array = event["affected"]
	var clues: Array = host.sudoku.get_clues()
	for i in range(81):
		var button: Button = host.cells[i]
		var value: int = solver.board[i]
		button.text = "" if value == 0 else str(value)
		var row: int = i / 9
		var col: int = i % 9
		var style: StyleBoxFlat = host._cell_style(row, col, false, false, false)
		if i in affected:
			style.bg_color = COLOR_AFFECTED
		if i == focused:
			style.bg_color = COLOR_CONFLICT if event["kind"] == "conflict" else COLOR_FOCUS
		for state in ["normal", "hover", "pressed", "disabled"]:
			button.add_theme_stylebox_override(state, style)
		var color := Color("edf2f7") if clues[i] else Color("74c5ff")
		button.add_theme_color_override("font_disabled_color", color)
		var notes: Control = host.note_layers[i]
		notes.visible = value == 0
		for digit in range(1, 10):
			var label := notes.get_child(digit - 1) as Label
			label.text = str(digit) if solver.domains[i] & (1 << (digit - 1)) else ""
