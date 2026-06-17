#include <cstdio>

#include <ui/ui.hpp>

#ifdef _WIN32
#include <windows.h>
#endif

namespace {

constexpr int kPopupCornerRadius = 12;
constexpr int kPopupWidth = 200;
constexpr int kPopupHeight = 40;

ui::DrawPath make_rounded_rectangle(double x, double y, double width, double height,
                                    double radius) {
  double r = radius;
  if (r > width / 2.0) {
    r = width / 2.0;
  }
  if (r > height / 2.0) {
    r = height / 2.0;
  }
  constexpr double kPi = 3.14159265358979323846;
  ui::DrawPath path = ui::DrawPath::make();
  path.new_figure(x + r, y)
      .line_to(x + width - r, y)
      .arc_to(x + width - r, y + r, r, 3.0 * kPi / 2.0, kPi / 2.0)
      .line_to(x + width, y + height - r)
      .arc_to(x + width - r, y + height - r, r, 0, kPi / 2.0)
      .line_to(x + r, y + height)
      .arc_to(x + r, y + height - r, r, kPi / 2.0, kPi / 2.0)
      .line_to(x, y + r)
      .arc_to(x + r, y + r, r, kPi, kPi / 2.0)
      .close_figure()
      .end();
  return path;
}

#ifdef _WIN32
void apply_rounded_window_shape(uiWindow *window, int width, int height, int corner_radius) {
  HWND hwnd = reinterpret_cast<HWND>(uiControlHandle(uiControl(window)));
  HRGN region = CreateRoundRectRgn(0, 0, width + 1, height + 1, corner_radius, corner_radius);
  if (region == nullptr) {
    return;
  }
  if (SetWindowRgn(hwnd, region, TRUE) == 0) {
    DeleteObject(region);
  }
}

void set_wnd_icon(const ui::Window &window, const char *icon_path) {
  if (icon_path == nullptr) {
    return;
  }
  HICON icon = LoadIcon(GetModuleHandle(nullptr), icon_path);
  if (icon == nullptr) {
    return;
  }
  SendMessage(window.handle<HWND>(), WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(icon));
  SendMessage(window.handle<HWND>(), WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(icon));
}

#endif

}  // namespace

struct PopupWindow {
  PopupWindow() {
    handler_.Draw = [](uiAreaHandler *handler, uiArea *area, uiAreaDrawParams *params) {
      printf("Draw\n");
      ui::DrawContext::wrap(params->Context)
          .fill(make_rounded_rectangle(0, 0, params->AreaWidth, params->AreaHeight,
                                       kPopupCornerRadius),
                ui::DrawBrush::solid(1.0, 0.0, 0.0, 1.0));
    };
    handler_.MouseEvent = [](uiAreaHandler *handler, uiArea *area, uiAreaMouseEvent *event) {
      printf("MouseEvent\n");
    };
    handler_.MouseCrossed = [](uiAreaHandler *handler, uiArea *area, int left) {
      printf("MouseCrossed\n");
    };
    handler_.DragBroken = [](uiAreaHandler *handler, uiArea *area) { printf("DragBroken\n"); };
    handler_.KeyEvent = [](uiAreaHandler *handler, uiArea *area, uiAreaKeyEvent *event) -> int {
      printf("KeyEvent\n");
      return 0;
    };
  }

  void close() {
    if (window_.raw() == nullptr) {
      return;
    }
    area_ = {};
    window_.destroy();
  }

  void show() {
    area_ = ui::Area::make(handler_);
    window_ = ui::Window::make("pop up", kPopupWidth, kPopupHeight, false)
                  .borderless(true)
                  .margined(false)
                  .resizable(false)
                  .set_child(area_);
    window_.on_focus_changed(on_focus_changed, this).show();
#ifdef _WIN32
    apply_rounded_window_shape(window_.raw(), kPopupWidth, kPopupHeight, kPopupCornerRadius);
#endif
  }

  void toggle() {
    if (window_.raw() != nullptr) {
      close();
      return;
    }
    show();
  }

  bool is_open() const { return window_.raw() != nullptr; }

 private:
  ui::Window window_{};
  ui::Area area_{};
  ui::AreaHandler handler_{};

  static void on_focus_changed(uiWindow *sender, void *data) {
    if (uiWindowFocused(sender) != 0) {
      return;
    }
    ui::Application::queue_main(
        [](void *queued_data) {
          auto *self = static_cast<PopupWindow *>(queued_data);
          if (self->window_.raw() != nullptr) {
            self->close();
          }
        },
        data);
  }
};

class BasicControlsPage {
 public:
  explicit BasicControlsPage(ui::Window &owner) : root_(build_root()) { (void)owner; }

  BasicControlsPage(const BasicControlsPage &) = delete;
  BasicControlsPage &operator=(const BasicControlsPage &) = delete;

  ui::VerticalBox root() const { return root_.copy(); }

 private:
  static ui::VerticalBox build_root() {
    return ui::VerticalBox::make()
        .append(ui::HorizontalBox::make()
                    .append(ui::Button::make("Button"))
                    .append(ui::Checkbox::make("Checkbox"))
                    .padded(true))
        .append(ui::Label::make("This is a label.\nLabels can span multiple lines."))
        .append(ui::Separator::make_horizontal())
        .append_stretchy(ui::Group::make("Entries").margined(true).set_child(
            ui::Form::make()
                .padded(true)
                .append("Entry", ui::Entry::make())
                .append("Password Entry", ui::Entry::make_password())
                .append("Search Entry", ui::Entry::make_search())
                .append_stretchy("Multiline Entry", ui::MultilineEntry::make())
                .append_stretchy("Multiline Entry No Wrap",
                                 ui::MultilineEntry::make_non_wrapping())))
        .padded(true);
  }

  ui::VerticalBox root_{};
};

class NumbersPage {
 public:
  explicit NumbersPage(ui::Window &owner) : root_() {
    (void)owner;
    build_layout();
  }

  NumbersPage(const NumbersPage &) = delete;
  NumbersPage &operator=(const NumbersPage &) = delete;

  ui::HorizontalBox root() const { return root_.copy(); }

 private:
  static void on_spinbox_changed(uiSpinbox *, void *data) {
    auto *self = static_cast<NumbersPage *>(data);
    int value = self->spinbox_.value();
    self->slider_.set_value(value);
    self->progress_.set_value(value);
  }

  static void on_slider_changed(uiSlider *, void *data) {
    auto *self = static_cast<NumbersPage *>(data);
    int value = self->slider_.value();
    self->spinbox_.set_value(value);
    self->progress_.set_value(value);
  }

  void build_layout() {
    root_ =
        ui::HorizontalBox::make()
            .padded(true)
            .append_stretchy(ui::Group::make("Numbers").margined(true).set_child(
                ui::VerticalBox::make()
                    .append(ui::Spinbox::make_into(spinbox_, 0, 100)
                                .on_changed(on_spinbox_changed, this))
                    .append(
                        ui::Slider::make_into(slider_, 0, 100).on_changed(on_slider_changed, this))
                    .append(ui::ProgressBar::make_into(progress_))
                    .append(ui::ProgressBar::make().set_value(-1))
                    .padded(true)))
            .append_stretchy(ui::Group::make("Lists").margined(true).set_child(
                ui::VerticalBox::make()
                    .append(ui::Combobox::make()
                                .append("Combobox Item 1")
                                .append("Combobox Item 2")
                                .append("Combobox Item 3"))
                    .append(ui::EditableCombobox::make()
                                .append("Editable Item 1")
                                .append("Editable Item 2")
                                .append("Editable Item 3"))
                    .append(ui::RadioButtons::make()
                                .append("Radio Button 1")
                                .append("Radio Button 2")
                                .append("Radio Button 3"))
                    .padded(true)));
  }

  ui::HorizontalBox root_{};
  ui::Spinbox spinbox_{};
  ui::Slider slider_{};
  ui::ProgressBar progress_{};
};

class DataChoosersPage {
 public:
  explicit DataChoosersPage(ui::Window &owner) : owner_(owner), root_() { build_layout(); }

  DataChoosersPage(const DataChoosersPage &) = delete;
  DataChoosersPage &operator=(const DataChoosersPage &) = delete;

  ui::HorizontalBox root() const { return root_.copy(); }

 private:
  static void on_open_file_clicked(uiButton *, void *data) {
    auto *self = static_cast<DataChoosersPage *>(data);
    ui::Text filename = ui::open_file(self->owner_);
    if (filename.empty()) {
      self->open_file_entry_.set_text("(cancelled)");
      return;
    }
    self->open_file_entry_.set_text(filename.c_str());
  }

  static void on_open_folder_clicked(uiButton *, void *data) {
    auto *self = static_cast<DataChoosersPage *>(data);
    ui::Text filename = ui::open_folder(self->owner_);
    if (filename.empty()) {
      self->open_folder_entry_.set_text("(cancelled)");
      return;
    }
    self->open_folder_entry_.set_text(filename.c_str());
  }

  static void on_save_file_clicked(uiButton *, void *data) {
    auto *self = static_cast<DataChoosersPage *>(data);
    ui::Text filename = ui::save_file(self->owner_);
    if (filename.empty()) {
      self->save_file_entry_.set_text("(cancelled)");
      return;
    }
    self->save_file_entry_.set_text(filename.c_str());
  }

  static void on_message_box_clicked(uiButton *, void *data) {
    auto *self = static_cast<DataChoosersPage *>(data);
    ui::msg_box(self->owner_, "This is a normal message box.",
                "More detailed information can be shown here.");
  }

  static void on_error_box_clicked(uiButton *, void *data) {
    auto *self = static_cast<DataChoosersPage *>(data);
    ui::msg_box_error(self->owner_, "This message box describes an error.",
                      "More detailed information can be shown here.");
  }

  void build_layout() {
    root_ =
        ui::HorizontalBox::make()
            .append(ui::VerticalBox::make()
                        .append(ui::DateTimePicker::make_date())
                        .append(ui::DateTimePicker::make_time())
                        .append(ui::DateTimePicker::make())
                        .append(ui::FontButton::make())
                        .append(ui::ColorButton::make())
                        .padded(true))
            .append(ui::Separator::make_vertical())
            .padded(true)
            .append_stretchy(
                ui::VerticalBox::make()
                    .append(
                        ui::Grid::make()
                            .padded(true)
                            .append(ui::Entry::make_into(open_file_entry_).readonly(true), 1, 0, 1,
                                    1, true, uiAlignFill, false, uiAlignFill)
                            .append(ui::Button::make_into(open_file_button_, "  Open File  ")
                                        .on_clicked(on_open_file_clicked, this),
                                    0, 0, 1, 1, false, uiAlignFill, false, uiAlignFill)
                            .append(ui::Entry::make_into(open_folder_entry_).readonly(true), 1, 1,
                                    1, 1, true, uiAlignFill, false, uiAlignFill)
                            .append(ui::Button::make_into(open_folder_button_, "Open Folder")
                                        .on_clicked(on_open_folder_clicked, this),
                                    0, 1, 1, 1, false, uiAlignFill, false, uiAlignFill)
                            .append(ui::Entry::make_into(save_file_entry_).readonly(true), 1, 2, 1,
                                    1, true, uiAlignFill, false, uiAlignFill)
                            .append(ui::Button::make_into(save_file_button_, "  Save File  ")
                                        .on_clicked(on_save_file_clicked, this),
                                    0, 2, 1, 1, false, uiAlignFill, false, uiAlignFill)
                            .append(
                                ui::Grid::make()
                                    .padded(true)
                                    .append(ui::Button::make_into(msg_box_button_, "Message Box")
                                                .on_clicked(on_message_box_clicked, this),
                                            0, 0, 1, 1, false, uiAlignFill, false, uiAlignFill)
                                    .append(ui::Button::make_into(error_box_button_, "Error Box")
                                                .on_clicked(on_error_box_clicked, this),
                                            1, 0, 1, 1, false, uiAlignFill, false, uiAlignFill),
                                0, 3, 2, 1, false, uiAlignCenter, false, uiAlignStart))
                    .padded(true));
  }

  ui::Window &owner_;
  ui::HorizontalBox root_{};

  ui::Button open_file_button_{};
  ui::Entry open_file_entry_{};
  ui::Button open_folder_button_{};
  ui::Entry open_folder_entry_{};
  ui::Button save_file_button_{};
  ui::Entry save_file_entry_{};
  ui::Button msg_box_button_{};
  ui::Button error_box_button_{};
};

class PopupPage {
 public:
  explicit PopupPage(ui::Window &owner) : root_() {
    (void)owner;
    build_layout();
  }

  PopupPage(const PopupPage &) = delete;
  PopupPage &operator=(const PopupPage &) = delete;

  ui::VerticalBox root() const { return root_.copy(); }

  void close_popup_if_open() { popup_.close(); }

 private:
  static void on_toggle_popup_clicked(uiButton *, void *data) {
    auto *self = static_cast<PopupPage *>(data);
    self->popup_.toggle();
  }

  void build_layout() {
    root_ = ui::VerticalBox::make().padded(true).append(
        ui::Group::make("Popup Window")
            .margined(true)
            .set_child(
                ui::VerticalBox::make()
                    .padded(true)
                    .append(ui::Label::make("Borderless popup that closes when clicking outside."))
                    .append(ui::Button::make("Toggle Popup")
                                .on_clicked(on_toggle_popup_clicked, this))));
  }

  ui::VerticalBox root_{};
  PopupWindow popup_{};
};

class GalleryApp {
 public:
  GalleryApp()
      : main_window_(ui::Window::make("libui Control Gallery", 640, 480, true)),
        tab_(ui::Tab::make()),
        basic_controls_page_(main_window_),
        numbers_page_(main_window_),
        data_choosers_page_(main_window_),
        popup_page_(main_window_) {
    bind_events();
    setup_layout();
  }

  GalleryApp(const GalleryApp &) = delete;
  GalleryApp &operator=(const GalleryApp &) = delete;

  void run() {
    main_window_.show();
    ui::Application::run();
  }

  static void setup_menus() {
    ui::Menu file_menu = ui::Menu::make("File");
    file_menu.append_item("Open").on_clicked(
        [](uiMenuItem *, uiWindow *window, void *) {
          ui::Window parent = ui::Window::wrap(window);
          ui::Text filename = ui::open_file(parent);
          if (filename.empty()) {
            ui::msg_box_error(parent, "No file selected", "Don't be alarmed!");
            return;
          }
          ui::msg_box(parent, "File selected", filename.c_str());
        },
        nullptr);
    file_menu.append_item("Open Folder")
        .on_clicked(
            [](uiMenuItem *, uiWindow *window, void *) {
              ui::Window parent = ui::Window::wrap(window);
              ui::Text filename = ui::open_folder(parent);
              if (filename.empty()) {
                ui::msg_box_error(parent, "No folder selected", "Don't be alarmed!");
                return;
              }
              ui::msg_box(parent, "Folder selected", filename.c_str());
            },
            nullptr);
    file_menu.append_item("Save").on_clicked(
        [](uiMenuItem *, uiWindow *window, void *) {
          ui::Window parent = ui::Window::wrap(window);
          ui::Text filename = ui::save_file(parent);
          if (filename.empty()) {
            ui::msg_box_error(parent, "No file selected", "Don't be alarmed!");
            return;
          }
          ui::msg_box(parent, "File selected (don't worry, it's still there)", filename.c_str());
        },
        nullptr);
    file_menu.append_quit_item();

    ui::Menu edit_menu = ui::Menu::make("Edit");
    edit_menu.append_check_item("Checkable Item");
    edit_menu.append_separator();
    edit_menu.append_item("Disabled Item").disable();
    edit_menu.append_preferences_item();

    ui::Menu help_menu = ui::Menu::make("Help");
    help_menu.append_item("Help");
    help_menu.append_about_item();
  }

 private:
  void bind_events() {
    ui::Application::on_should_quit(
        [](void *data) {
          auto *self = static_cast<GalleryApp *>(data);
          self->popup_page_.close_popup_if_open();
          self->main_window_.destroy();
          return 1;
        },
        this);
    main_window_.on_closing(
        [](uiWindow *, void *data) {
          auto *self = static_cast<GalleryApp *>(data);
          self->popup_page_.close_popup_if_open();
          self->main_window_.destroy();
          ui::Application::quit();
          return 0;
        },
        this);
  }

  void setup_layout() {
    main_window_.set_child(tab_);
    main_window_.margined(true);

    tab_.append("Basic Controls", basic_controls_page_.root());
    tab_.set_margined(0, true);
    tab_.append("Numbers and Lists", numbers_page_.root());
    tab_.set_margined(1, true);
    tab_.append("Data Choosers", data_choosers_page_.root());
    tab_.set_margined(2, true);
    tab_.append("Popup Window", popup_page_.root());
    tab_.set_margined(3, true);
  }

  ui::Window main_window_{};
  ui::Tab tab_{};
  BasicControlsPage basic_controls_page_;
  NumbersPage numbers_page_;
  DataChoosersPage data_choosers_page_;
  PopupPage popup_page_;
};

int main() {
  std::string err;
  if (!ui::Application::init(&err)) {
    fprintf(stderr, "error initializing libui: %s\n", err.c_str());
    return 1;
  }

  // Menus must be created before windows are instantiated on Windows backend.
  GalleryApp::setup_menus();
  {
    GalleryApp app;
    app.run();
  }

  ui::Application::uninit();
  return 0;
}
