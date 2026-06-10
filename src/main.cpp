#include <cstdio>

#include <ui.hpp>

struct AppContext {
  ui::Window *mainwin;
};

static ui::Spinbox g_spinbox;
static ui::Slider g_slider;
static ui::ProgressBar g_progress;

static ui::Button g_open_file_button;
static ui::Entry g_open_file_entry;
static ui::Button g_open_folder_button;
static ui::Entry g_open_folder_entry;
static ui::Button g_save_file_button;
static ui::Entry g_save_file_entry;
static ui::Button g_msg_box_button;
static ui::Button g_error_box_button;

static ui::Window *g_mainwin;

static int on_closing(uiWindow *sender, void *data) {
  (void)sender;
  ui::Application::quit();
  return 1;
}

static int on_should_quit(void *data) {
  static_cast<AppContext *>(data)->mainwin->destroy();
  return 1;
}

static void on_spinbox_changed(uiSpinbox *sender, void *) {
  (void)sender;
  int value = g_spinbox.value();
  g_slider.set_value(value);
  g_progress.set_value(value);
}

static void on_slider_changed(uiSlider *sender, void *) {
  (void)sender;
  int value = g_slider.value();
  g_spinbox.set_value(value);
  g_progress.set_value(value);
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
          ui::VerticalBox::make(
              ui::Spinbox::make_into(g_spinbox, 0, 100).on_changed(on_spinbox_changed, nullptr),
              ui::Slider::make_into(g_slider, 0, 100).on_changed(on_slider_changed, nullptr),
              ui::ProgressBar::make_into(g_progress), ui::ProgressBar::make().set_value(-1))
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

static void on_open_file_clicked(uiButton *sender, void *data) {
  (void)sender;
  ui::Entry *entry = static_cast<ui::Entry *>(data);
  printf("sender: %p, data: %p, entry_raw: %p\n", sender, data, entry->raw());

  ui::Text filename = ui::open_file(*g_mainwin);
  if (filename.empty()) {
    entry->set_text("(cancelled)");
    return;
  }
  entry->set_text(filename.c_str());
}

static void on_open_folder_clicked(uiButton *sender, void *data) {
  (void)sender;
  printf("sender: %p, data: %p\n", sender, data);

  ui::Entry *entry = static_cast<ui::Entry *>(data);
  ui::Text filename = ui::open_folder(*g_mainwin);
  if (filename.empty()) {
    entry->set_text("(cancelled)");
    return;
  }
  entry->set_text(filename.c_str());
}

static void on_save_file_clicked(uiButton *sender, void *data) {
  (void)sender;
  printf("sender: %p, data: %p\n", sender, data);
  ui::Entry *entry = static_cast<ui::Entry *>(data);
  ui::Text filename = ui::save_file(*g_mainwin);
  if (filename.empty()) {
    entry->set_text("(cancelled)");
    return;
  }
  entry->set_text(filename.c_str());
}

static void on_msg_box_clicked(uiButton *sender, void *) {
  (void)sender;
  ui::msg_box(*g_mainwin, "This is a normal message box.",
              "More detailed information can be shown here.");
}

static void on_msg_box_error_clicked(uiButton *sender, void *) {
  (void)sender;
  ui::msg_box_error(*g_mainwin, "This message box describes an error.",
                    "More detailed information can be shown here.");
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
                  .append(ui::Entry::make_into(g_open_file_entry).readonly(true), 1, 0, 1, 1, true,
                          uiAlignFill, false, uiAlignFill)
                  .append(ui::Button::make_into(g_open_file_button, "  Open File  ")
                              .on_clicked(on_open_file_clicked, &g_open_file_entry),
                          0, 0, 1, 1, false, uiAlignFill, false, uiAlignFill)
                  .append(ui::Entry::make_into(g_open_folder_entry).readonly(true), 1, 1, 1, 1,
                          true, uiAlignFill, false, uiAlignFill)
                  .append(ui::Button::make_into(g_open_folder_button, "Open Folder")
                              .on_clicked(on_open_folder_clicked, &g_open_folder_entry),
                          0, 1, 1, 1, false, uiAlignFill, false, uiAlignFill)
                  .append(ui::Entry::make_into(g_save_file_entry).readonly(true), 1, 2, 1, 1, true,
                          uiAlignFill, false, uiAlignFill)
                  .append(ui::Button::make_into(g_save_file_button, "  Save File  ")
                              .on_clicked(on_save_file_clicked, &g_save_file_entry),
                          0, 2, 1, 1, false, uiAlignFill, false, uiAlignFill)
                  .append(ui::Grid::make()
                              .padded(true)
                              .append(ui::Button::make_into(g_msg_box_button, "Message Box")
                                          .on_clicked(on_msg_box_clicked, nullptr),
                                      0, 0, 1, 1, false, uiAlignFill, false, uiAlignFill)
                              .append(ui::Button::make_into(g_error_box_button, "Error Box")
                                          .on_clicked(on_msg_box_error_clicked, nullptr),
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
    g_mainwin = &mainwin;

    AppContext ctx = {&mainwin};
    mainwin.on_closing(on_closing, &ctx);
    ui::Application::on_should_quit(on_should_quit, &ctx);

    ui::Tab tab = ui::Tab::make();
    mainwin.set_child(tab);
    mainwin.margined(true);

    tab.append("Basic Controls", make_basic_controls_page());
    tab.set_margined(0, true);
    tab.append("Numbers and Lists", make_numbers_page());
    tab.set_margined(1, true);
    tab.append("Data Choosers", make_data_choosers_page());
    tab.set_margined(2, true);

    mainwin.show();
    ui::Application::run();
  }
  ui::Application::uninit();
  return 0;
}
