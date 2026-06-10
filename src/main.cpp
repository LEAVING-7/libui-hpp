#include <cstdio>

#include <ui.hpp>

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
  uiWindow *popup = nullptr;
};

struct PopupDrawState {
  uiAttributedString *text = nullptr;
  uiFontDescriptor font{};
  uiDrawTextLayout *layout = nullptr;
  double layout_width = -1;
};

static GalleryApp g_app;
static PopupDrawState g_popup_draw;

static void free_popup_draw_state() {
  if (g_popup_draw.layout != nullptr) {
    uiDrawFreeTextLayout(g_popup_draw.layout);
    g_popup_draw.layout = nullptr;
  }
  if (g_popup_draw.text != nullptr) {
    uiFreeAttributedString(g_popup_draw.text);
    g_popup_draw.text = nullptr;
  }
  if (g_popup_draw.font.Family != nullptr) {
    uiFreeFontDescriptor(&g_popup_draw.font);
    g_popup_draw.font = {};
  }
  g_popup_draw.layout_width = -1;
}

static void init_popup_draw_state() {
  free_popup_draw_state();
  g_popup_draw.text = uiNewAttributedString("Hello, World!");
  uiLoadControlFont(&g_popup_draw.font);
}

static void ensure_popup_layout(double width) {
  if (g_popup_draw.layout != nullptr && g_popup_draw.layout_width == width) {
    return;
  }
  if (g_popup_draw.layout != nullptr) {
    uiDrawFreeTextLayout(g_popup_draw.layout);
    g_popup_draw.layout = nullptr;
  }

  uiDrawTextLayoutParams layout_params{};
  layout_params.String = g_popup_draw.text;
  layout_params.DefaultFont = &g_popup_draw.font;
  layout_params.Width = width;
  layout_params.Align = uiDrawTextAlignCenter;
  g_popup_draw.layout = uiDrawNewTextLayout(&layout_params);
  g_popup_draw.layout_width = width;
}

static void popup_surface_draw(uiAreaHandler *, uiArea *, uiAreaDrawParams *params) {
  uiDrawPath *path = uiDrawNewPath(uiDrawFillModeWinding);
  uiDrawPathAddRectangle(path, 0, 0, params->AreaWidth, params->AreaHeight);
  uiDrawPathEnd(path);

  uiDrawBrush brush{};
  brush.Type = uiDrawBrushTypeSolid;
  brush.R = 1.0;
  brush.G = 1.0;
  brush.B = 1.0;
  brush.A = 1.0;

  uiDrawFill(params->Context, path, &brush);
  uiDrawFreePath(path);

  if (g_popup_draw.text == nullptr) {
    return;
  }

  ensure_popup_layout(params->AreaWidth);
  double width = 0;
  double height = 0;
  uiDrawTextLayoutExtents(g_popup_draw.layout, &width, &height);
  uiDrawText(params->Context, g_popup_draw.layout, (params->AreaWidth - width) / 2,
             (params->AreaHeight - height) / 2);
}

static void popup_surface_mouse(uiAreaHandler *, uiArea *area, uiAreaMouseEvent *event) {
  if (event->Down != 0) {
    uiAreaBeginUserWindowMove(area);
  }
}

static void popup_surface_mouse_crossed(uiAreaHandler *, uiArea *, int) {}

static void popup_surface_drag_broken(uiAreaHandler *, uiArea *) {}

static int popup_surface_key(uiAreaHandler *, uiArea *, uiAreaKeyEvent *) { return 0; }

static uiAreaHandler g_popup_surface_handler = {
    popup_surface_draw,
    popup_surface_mouse,
    popup_surface_mouse_crossed,
    popup_surface_drag_broken,
    popup_surface_key,
};

static void close_popup() {
  if (g_app.popup == nullptr) {
    return;
  }
  uiWindow *popup = g_app.popup;
  g_app.popup = nullptr;
  ui::Window::wrap(popup).destroy();
  free_popup_draw_state();
}

static void show_popup() {
  init_popup_draw_state();

  ui::Window popup = ui::Window::make("pop up", 500, 500, true);
  g_app.popup = popup.raw();

  uiArea *surface = uiNewArea(&g_popup_surface_handler);

  popup.borderless(true)
      .margined(true)
      .resizable(true);
  uiWindowSetChild(popup.raw(), uiControl(surface));
  popup
      .on_focus_changed(
          [](uiWindow *sender, void *) {
            if (uiWindowFocused(sender) != 0) {
              return;
            }
            ui::Application::queue_main(
                [](void *data) {
                  uiWindow *w = static_cast<uiWindow *>(data);
                  if (g_app.popup == w) {
                    close_popup();
                  }
                },
                sender);
          },
          nullptr)
      .show();
}

static void toggle_popup() {
  if (g_app.popup != nullptr) {
    close_popup();
    return;
  }
  show_popup();
}

static ui::VerticalBox make_popup_page() {
  return ui::VerticalBox::make(
             ui::Group::make("Popup Window")
                 .margined(true)
                 .set_child(ui::VerticalBox::make(
                     ui::Label::make("Borderless popup that closes when clicking outside.\n"
                                     "Drag anywhere inside to move the window."),
                     ui::Button::make("Toggle Popup").on_clicked(
                         [](uiButton *, void *) { toggle_popup(); }, nullptr))
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
              .append("Multiline Entry", ui::MultilineEntry::make(), true)
              .append("Multiline Entry No Wrap", ui::MultilineEntry::make_non_wrapping(), true)))
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
          close_popup();
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
