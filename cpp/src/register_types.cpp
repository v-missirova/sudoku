#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/godot.hpp>
#include "sudoku_node.h"

using namespace godot;

void gdextension_initialize(ModuleInitializationLevel p_level) {
    if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) return;
    ClassDB::register_class<SudokuNode>();
}

void gdextension_terminate(ModuleInitializationLevel p_level) {
}

extern "C" {
GDExtensionBool GDE_EXPORT sudoku_library_init(
    GDExtensionInterfaceGetProcAddress p_get_proc_address,
    GDExtensionClassLibraryPtr        p_library,
    GDExtensionInitialization        *r_initialization)
{
    GDExtensionBinding::InitObject init_obj(
        p_get_proc_address, p_library, r_initialization);

    init_obj.register_initializer(gdextension_initialize);
    init_obj.register_terminator(gdextension_terminate);
    init_obj.set_minimum_library_initialization_level(
        MODULE_INITIALIZATION_LEVEL_SCENE);

    return init_obj.init();
}
}

