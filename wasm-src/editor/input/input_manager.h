/**
 * @file input_manager.h
 * @brief Contains the declaration of the InputManager class.
 */

#pragma once

#include "hover_state.h"
#include "tool.h"

namespace graphick::editor {
class Entity;
};

namespace graphick::editor::input {

/**
 * @brief The InputManager class is a singleton class that manages input events.
 *
 * This class provides methods for handling pointer, keyboard, and clipboard events.
 * Events are automatically dispatched to the appropriate tool.
 * All of the events will be handled referring to the scene is currently active.
 *
 * @class InputManager
 */
class InputManager {
public:
  /**
   * @brief The PointerEvent enum represents the type of pointer event.
   */
  enum class PointerEvent { Down = 0, Move = 1, Up = 2, Enter = 3, Leave = 4 };

  /**
   * @brief The PointerButton enum represents the mouse button, touch or pen identifier.
   *
   * If the pointer type is touch, the button will be always PointerButton::Left.
   * PointerType::Eraser is only available for pen devices.
   */
  enum class PointerButton {
    Left = 0,
#ifdef EMSCRIPTEN
    Middle = 1,
    Right = 2,
#else
    Right = 1,
    Middle = 2,
#endif
    Eraser = 5
  };

  /**
   * @brief The PointerTarget enum represents the target of the pointer event.
   */
  enum class PointerTarget { Other = 0, Canvas = 1 };

  /**
   * @brief The PointerType enum represents the type of pointer.
   */
  enum class PointerType { Mouse = 0, Pen = 1, Touch = 2 };

  /**
   * @brief The KeyboardKey enum represents whether the key is pressed or released.
   */
  enum class KeyboardEvent { Down = 0, Up = 1 };

  /**
   * @brief The ClipboardEvent enum represents the type of clipboard event.
   */
  enum class ClipboardEvent { Copy = 0, Paste = 1, Cut = 2 };
public:
  /**
   * @brief Groups the pointer coordinates.
   *
   * @struct PointerCoord
   */
  struct PointerCoord {
    vec2 position{};  // The position of the pointer.
    vec2 movement{};  // The difference between the current and the previous position.
    vec2 delta{};     // The difference between the current and the initial position.
    vec2 origin{};    // The initial position of the pointer.
  };

  /**
   * @brief Groups the state of the keys.
   *
   * The ctrl, alt, and shift fields represent the state of the keys.
   * The ctrl_state_changed, alt_state_changed, and shift_state_changed fields represent whether
   * the state of the keys has changed between the previous and the current frame.
   *
   * @struct KeysState
   */
  struct KeysState {
    bool ctrl = false;
    bool alt = false;
    bool shift = false;
    bool space = false;
    bool ctrl_state_changed = false;
    bool alt_state_changed = false;
    bool shift_state_changed = false;
    bool space_state_changed = false;
  };

  /**
   * @brief Represents the full state of a pointer.
   *
   * @struct Pointer
   */
  struct Pointer {
    PointerTarget target = PointerTarget::Other;  // The target that the pointer is over.

    PointerCoord client{};                        // The client-space coordinates of the pointer.
    PointerCoord scene{};                         // The scene-space coordinates of the pointer.

    bool down = false;                            // Whether the pointer is down.
    bool inside = true;                           // Whether the pointer is inside the canvas.
    float pressure = 1.0f;                        // The pressure of the pointer, 1.0 if not available (i.e. mouse).
    double time = 0.0f;                           // The time stamp of the last pointer update.

    PointerType type = PointerType::Mouse;        // The type of the pointer.
    PointerButton button = PointerButton::Left;   // The button of the pointer.
  };
public:
  static KeysState keys;                          // The state of the keys.
  static Pointer pointer;                         // The state of the pointer.
  static HoverState hover;                        // The hover state of the pointer.
public:
  /**
   * @brief Deleted copy and move constructors.
   */
  InputManager(const InputManager&) = delete;
  InputManager(InputManager&&) = delete;

  /**
   * @brief Gets the static instance of the InputManager class.
   *
   * @return The InputManager instance.
   */
  static inline InputManager* get() { return s_instance; }

  /**
   * @brief Initializes the InputManager class.
   */
  static void init();

  /**
   * @brief Shuts down the InputManager class.
   */
  static void shutdown();

  /**
   * @brief Handles a pointer event.
   *
   * @param target The target of the pointer event.
   * @param event The type of the pointer event.
   * @param type The type of the pointer.
   * @param button The button of the pointer.
   * @param x The x-coordinate of the pointer in client-space.
   * @param y The y-coordinate of the pointer in client-space.
   * @param pressure The pressure of the pointer.
   * @param time_stamp The time stamp of the pointer event.
   * @param alt Whether the alt key is pressed.
   * @param ctrl Whether the ctrl key is pressed.
   * @param shift Whether the shift key is pressed.
   * @return Whether the event was handled.
   */
  static bool on_pointer_event(
    PointerTarget target,
    PointerEvent event,
    PointerType type,
    PointerButton button,
    float x,
    float y,
    float pressure,
    double time_stamp,
    bool alt,
    bool ctrl,
    bool shift
  );

  /**
   * @brief Handles a keyboard event.
   *
   * @param event The type of the keyboard event.
   * @param key The key that was pressed or released.
   * @param repeat Whether the key is being held down.
   * @param alt Whether the alt key is pressed.
   * @param ctrl Whether the ctrl key is pressed.
   * @param shift Whether the shift key is pressed.
   * @return Whether the event was handled.
   */
  static bool on_keyboard_event(KeyboardEvent event, KeyboardKey key, bool repeat, bool alt, bool ctrl, bool shift);

  /**
   * @brief Handles a resize event.
   *
   * @param width The width of the canvas.
   * @param height The height of the canvas.
   * @param dpr The device pixel ratio.
   * @param offset_x The x-coordinate of the canvas.
   * @param offset_y The y-coordinate of the canvas.
   * @return Whether the event was handled.
   */
  static bool on_resize_event(int width, int height, float dpr, int offset_x, int offset_y);

  /**
   * @brief Handles a wheel event.
   *
   * @param target The target of the wheel event.
   * @param delta_x The horizontal delta of the wheel event.
   * @param delta_y The vertical delta of the wheel event.
   * @param ctrl Whether the ctrl key is pressed.
   * @return Whether the event was handled.
   */
  static bool on_wheel_event(PointerTarget target, float delta_x, float delta_y, bool ctrl);

  /**
   * @brief Handles a clipboard event.
   *
   * @param event The type of the clipboard event.
   * @return Whether the event was handled.
   */
  static bool on_clipboard_event(ClipboardEvent event);

  /**
   * @brief Handles a pinch event.
   *
   * @param target The target of the pinch event.
   * @param delta The delta of the pinch event.
   * @param center_x The x-coordinate of the center of the pinch event.
   * @param center_y The y-coordinate of the center of the pinch event.
   * @return Whether the event was handled.
   */
  static bool on_touch_pinch(PointerTarget target, float delta, float center_x, float center_y);

  /**
   * @brief Handles a drag event.
   *
   * @param target The target of the drag event.
   * @param delta_x The horizontal delta of the drag event.
   * @param delta_y The vertical delta of the drag event.
   * @return Whether the event was handled.
   */
  static bool on_touch_drag(PointerTarget target, float delta_x, float delta_y);

  /**
   * @brief Gets the current tool.
   *
   * @return The current tool.
   */
  static const Tool& tool();

  /**
   * @brief Sets the current tool to the specified type.
   *
   * @param type The type of the tool to set.
   */
  static void set_tool(Tool::ToolType type);
private:
  /**
   * @brief Default constructor and destructor.
   */
  InputManager() = default;
  ~InputManager() = default;

  /**
   * @brief Sets the state of the keys.
   *
   * @param alt Whether the alt key is pressed.
   * @param ctrl Whether the ctrl key is pressed.
   * @param shift Whether the shift key is pressed.
   */
  void set_keys_state(bool alt, bool ctrl, bool shift);

  /**
   * @brief Recalculates the hover state.
   */
  void recalculate_hover();

  /**
   * @brief Handles a pointer down event.
   *
   * @param target The target of the pointer event.
   * @param button The button of the pointer.
   * @param x The x-coordinate of the pointer in client-space.
   * @param y The y-coordinate of the pointer in client-space.
   * @return Whether the event was handled.
   */
  bool on_pointer_down(PointerTarget target, PointerButton button, float x, float y);

  /**
   * @brief Handles a pointer move event.
   *
   * @param target The target of the pointer event.
   * @param x The x-coordinate of the pointer in client-space.
   * @param y The y-coordinate of the pointer in client-space.
   * @return Whether the event was handled.
   */
  bool on_pointer_move(PointerTarget target, float x, float y);

  /**
   * @brief Handles a pointer up event.
   *
   * @return Whether the event was handled.
   */
  bool on_pointer_up();

  /**
   * @brief Handles a pointer enter event.
   *
   * @return Whether the event was handled.
   */
  bool on_pointer_enter();

  /**
   * @brief Handles a pointer leave event.
   *
   * @return Whether the event was handled.
   */
  bool on_pointer_leave();

  /**
   * @brief Handles a key down event.
   *
   * @param key The key that was pressed.
   * @return Whether the event was handled.
   */
  bool on_key_down(KeyboardKey key);

  /**
   * @brief Handles a key up event.
   *
   * @param key The key that was released.
   * @return Whether the event was handled.
   */
  bool on_key_up(KeyboardKey key);

  /**
   * @brief Handles a resize event.
   *
   * @param x The x-coordinate of the canvas.
   * @param y The y-coordinate of the canvas.
   * @param dpr The device pixel ratio.
   * @param offset_x The x-coordinate of the canvas.
   * @param offset_y The y-coordinate of the canvas.
   * @return Whether the event was handled.
   */
  bool on_resize(int x, int y, float dpr, int offset_x, int offset_y);

  /**
   * @brief Handles a wheel event.
   *
   * @param target The target of the wheel event.
   * @param delta_x The horizontal delta of the wheel event.
   * @param delta_y The vertical delta of the wheel event.
   * @param ctrl Whether the ctrl key is pressed.
   * @return Whether the event was handled.
   */
  bool on_wheel(PointerTarget target, float delta_x, float delta_y, bool ctrl);

  /**
   * @brief Handles a clipboard copy event.
   *
   * @return Whether the event was handled.
   */
  bool on_clipboard_copy();

  /**
   * @brief Handles a clipboard paste event.
   *
   * @return Whether the event was handled.
   */
  bool on_clipboard_paste();

  /**
   * @brief Handles a clipboard cut event.
   *
   * @return Whether the event was handled.
   */
  bool on_clipboard_cut();

  /**
   * @brief Handles a pinch event.
   *
   * @param target The target of the pinch event.
   * @param delta The delta of the pinch event.
   * @param center_x The x-coordinate of the center of the pinch event.
   * @param center_y The y-coordinate of the center of the pinch event.
   * @return Whether the event was handled.
   */
  bool on_pinch(PointerTarget target, float delta, float center_x, float center_y);

  /**
   * @brief Handles a drag event.
   *
   * @param target The target of the drag event.
   * @param delta_x The horizontal delta of the drag event.
   * @param delta_y The vertical delta of the drag event.
   * @return Whether the event was handled.
   */
  bool on_drag(PointerTarget target, float delta_x, float delta_y);
private:
  bool m_moving = false;            // Whether the pointer is moving.
  bool m_abort = false;             // Whether the pointer event should be aborted.
private:
  static InputManager* s_instance;  // The static instance of the InputManager singleton.
};

}  // namespace graphick::editor::input
