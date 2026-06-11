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

struct GalleryApp {
  ui::Spinbox spinbox;
  ui::Slider slider;
  ui::ProgressBar progress;

  ui::Button open_file_button;
  ui::Entry open_file_entry;
  ui::Button open_folder_button;
  ui::Entry open_folder_entry;
  ui::Button save_file_button;
  ui::Entry save_file_entry;
  ui::Button msg_box_button;
  ui::Button error_box_button;

  ui::Window *mainwin = nullptr;
  PopupWindow popup;
};

static GalleryApp g_app;

static ui::VerticalBox make_popup_page() {
  return ui::VerticalBox::make(
             ui::Group::make("Popup Window")
                 .margined(true)
                 .set_child(
                     ui::VerticalBox::make(
                         ui::Label::make("Borderless popup that closes when clicking outside."),
                         ui::Button::make("Toggle Popup")
                             .on_clicked([](uiButton *, void *) { g_app.popup.toggle(); }, nullptr))
                         .padded(true)))
      .padded(true);
}

static ui::VerticalBox make_basic_controls_page() {
  return ui::VerticalBox::make(
             ui::HorizontalBox::make(ui::Button::make("Button"), ui::Checkbox::make("Checkbox"))
                 .padded(true),
             ui::Label::make("This is a label.\nLabels can span multiple lines."),
             ui::Separator::make_horizontal())
      .append_stretchy(ui::Group::make("Entries").margined(true).set_child(
          ui::Form::make()
              .padded(true)
              .append("Entry", ui::Entry::make())
              .append("Password Entry", ui::Entry::make_password())
              .append("Search Entry", ui::Entry::make_search())
              .append_stretchy("Multiline Entry", ui::MultilineEntry::make())
              .append_stretchy("Multiline Entry No Wrap", ui::MultilineEntry::make_non_wrapping())))
      .padded(true);
}

static ui::HorizontalBox make_numbers_page() {
  return ui::HorizontalBox::make()
      .append_stretchy(ui::Group::make("Numbers").margined(true).set_child(
          ui::VerticalBox::make(ui::Spinbox::make_into(g_app.spinbox, 0, 100)
                                    .on_changed(
                                        [](uiSpinbox *sender, void *) {
                                          (void)sender;
                                          int value = g_app.spinbox.value();
                                          g_app.slider.set_value(value);
                                          g_app.progress.set_value(value);
                                        },
                                        nullptr),
                                ui::Slider::make_into(g_app.slider, 0, 100)
                                    .on_changed(
                                        [](uiSlider *sender, void *) {
                                          (void)sender;
                                          int value = g_app.slider.value();
                                          g_app.spinbox.set_value(value);
                                          g_app.progress.set_value(value);
                                        },
                                        nullptr),
                                ui::ProgressBar::make_into(g_app.progress),
                                ui::ProgressBar::make().set_value(-1))
              .padded(true)))
      .append_stretchy(ui::Group::make("Lists").margined(true).set_child(
          ui::VerticalBox::make(ui::Combobox::make()
                                    .append("Combobox Item 1")
                                    .append("Combobox Item 2")
                                    .append("Combobox Item 3"),
                                ui::EditableCombobox::make()
                                    .append("Editable Item 1")
                                    .append("Editable Item 2")
                                    .append("Editable Item 3"),
                                ui::RadioButtons::make()
                                    .append("Radio Button 1")
                                    .append("Radio Button 2")
                                    .append("Radio Button 3"))
              .padded(true)))
      .padded(true);
}

static ui::HorizontalBox make_data_choosers_page() {
  return ui::HorizontalBox::make(
             ui::VerticalBox::make(ui::DateTimePicker::make_date(), ui::DateTimePicker::make_time(),
                                   ui::DateTimePicker::make(), ui::FontButton::make(),
                                   ui::ColorButton::make())
                 .padded(true),
             ui::Separator::make_vertical())
      .append_stretchy(
          ui::VerticalBox::make(
              ui::Grid::make()
                  .padded(true)
                  .append(ui::Entry::make_into(g_app.open_file_entry).readonly(true), 1, 0, 1, 1,
                          true, uiAlignFill, false, uiAlignFill)
                  .append(ui::Button::make_into(g_app.open_file_button, "  Open File  ")
                              .on_clicked(
                                  [](uiButton *sender, void *data) {
                                    ui::Entry *entry = static_cast<ui::Entry *>(data);
                                    printf("sender: %p, data: %p, entry_raw: %p\n", sender, data,
                                           entry->raw());
                                    ui::Text filename = ui::open_file(*g_app.mainwin);
                                    if (filename.empty()) {
                                      entry->set_text("(cancelled)");
                                      return;
                                    }
                                    entry->set_text(filename.c_str());
                                  },
                                  &g_app.open_file_entry),
                          0, 0, 1, 1, false, uiAlignFill, false, uiAlignFill)
                  .append(ui::Entry::make_into(g_app.open_folder_entry).readonly(true), 1, 1, 1, 1,
                          true, uiAlignFill, false, uiAlignFill)
                  .append(ui::Button::make_into(g_app.open_folder_button, "Open Folder")
                              .on_clicked(
                                  [](uiButton *sender, void *data) {
                                    printf("sender: %p, data: %p\n", sender, data);
                                    ui::Entry *entry = static_cast<ui::Entry *>(data);
                                    ui::Text filename = ui::open_folder(*g_app.mainwin);
                                    if (filename.empty()) {
                                      entry->set_text("(cancelled)");
                                      return;
                                    }
                                    entry->set_text(filename.c_str());
                                  },
                                  &g_app.open_folder_entry),
                          0, 1, 1, 1, false, uiAlignFill, false, uiAlignFill)
                  .append(ui::Entry::make_into(g_app.save_file_entry).readonly(true), 1, 2, 1, 1,
                          true, uiAlignFill, false, uiAlignFill)
                  .append(ui::Button::make_into(g_app.save_file_button, "  Save File  ")
                              .on_clicked(
                                  [](uiButton *sender, void *data) {
                                    (void)sender;
                                    ui::Entry *entry = static_cast<ui::Entry *>(data);
                                    ui::Text filename = ui::save_file(*g_app.mainwin);
                                    if (filename.empty()) {
                                      entry->set_text("(cancelled)");
                                      return;
                                    }
                                    entry->set_text(filename.c_str());
                                  },
                                  &g_app.save_file_entry),
                          0, 2, 1, 1, false, uiAlignFill, false, uiAlignFill)
                  .append(ui::Grid::make()
                              .padded(true)
                              .append(ui::Button::make_into(g_app.msg_box_button, "Message Box")
                                          .on_clicked(
                                              [](uiButton *sender, void *) {
                                                (void)sender;
                                                ui::msg_box(
                                                    *g_app.mainwin, "This is a normal message box.",
                                                    "More detailed information can be shown here.");
                                              },
                                              nullptr),
                                      0, 0, 1, 1, false, uiAlignFill, false, uiAlignFill)
                              .append(ui::Button::make_into(g_app.error_box_button, "Error Box")
                                          .on_clicked(
                                              [](uiButton *sender, void *) {
                                                (void)sender;
                                                ui::msg_box_error(
                                                    *g_app.mainwin,
                                                    "This message box describes an error.",
                                                    "More detailed information can be shown here.");
                                              },
                                              nullptr),
                                      1, 0, 1, 1, false, uiAlignFill, false, uiAlignFill),
                          0, 3, 2, 1, false, uiAlignCenter, false, uiAlignStart))
              .padded(true))
      .padded(true);
}

int main() {
  std::string err;
  if (!ui::Application::init(&err)) {
    fprintf(stderr, "error initializing libui: %s\n", err.c_str());
    return 1;
  }

  {
    ui::Window mainwin = ui::Window::make("libui Control Gallery", 640, 480, true);
    g_app.mainwin = &mainwin;

    mainwin.on_closing(
        [](uiWindow *, void *) {
          ui::Application::quit();
          return 1;
        },
        nullptr);
    ui::Application::on_should_quit(
        [](void *) {
          g_app.popup.close();
          g_app.mainwin->destroy();
          return 1;
        },
        nullptr);

    ui::Tab tab = ui::Tab::make();
    mainwin.set_child(tab);
    mainwin.margined(true);

    tab.append("Basic Controls", make_basic_controls_page());
    tab.set_margined(0, true);
    tab.append("Numbers and Lists", make_numbers_page());
    tab.set_margined(1, true);
    tab.append("Data Choosers", make_data_choosers_page());
    tab.set_margined(2, true);
    tab.append("Popup Window", make_popup_page());
    tab.set_margined(3, true);

    mainwin.show();

    ui::Application::run();
  }
  ui::Application::uninit();
  return 0;
}
