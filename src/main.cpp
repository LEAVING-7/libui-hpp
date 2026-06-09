#include <stdio.h>

#include <ui.hpp>

struct AppContext {
  ui::Application *app;
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
  static_cast<AppContext *>(data)->app->quit();
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
  return ui::VerticalBox::New(
      ui::HorizontalBox::New(
          ui::Button::New("Button"),
          ui::Checkbox::New("Checkbox"))
          .padded(true),
      ui::Label::New("This is a label.\nLabels can span multiple lines."),
      ui::Separator::NewHorizontal())
      .padded(true)
      .append(
          ui::Group::New("Entries").margined(true).set_child(
              ui::Form::New()
                  .padded(true)
                  .append("Entry", ui::Entry::New())
                  .append("Password Entry", ui::Entry::NewPassword())
                  .append("Search Entry", ui::Entry::NewSearch())
                  .append("Multiline Entry", ui::MultilineEntry::New(), true)
                  .append("Multiline Entry No Wrap", ui::MultilineEntry::NewNonWrapping(), true)),
          true);
}

static ui::HorizontalBox make_numbers_page() {
  g_spinbox = ui::Spinbox::New(0, 100).on_changed(on_spinbox_changed, nullptr);
  g_slider = ui::Slider::New(0, 100).on_changed(on_slider_changed, nullptr);
  g_progress = ui::ProgressBar::New();

  return ui::HorizontalBox::New()
      .padded(true)
      .append(ui::Group::New("Numbers").margined(true).set_child(ui::VerticalBox::New(
                  g_spinbox,
                  g_slider,
                  g_progress,
                  ui::ProgressBar::New().set_value(-1))
                                                                     .padded(true)),
              true)
      .append(ui::Group::New("Lists").margined(true).set_child(ui::VerticalBox::New(
                  ui::Combobox::New()
                      .append("Combobox Item 1")
                      .append("Combobox Item 2")
                      .append("Combobox Item 3"),
                  ui::EditableCombobox::New()
                      .append("Editable Item 1")
                      .append("Editable Item 2")
                      .append("Editable Item 3"),
                  ui::RadioButtons::New()
                      .append("Radio Button 1")
                      .append("Radio Button 2")
                      .append("Radio Button 3"))
                                                                   .padded(true)),
              true);
}

static void on_open_file_clicked(uiButton *sender, void *data) {
  (void)sender;
  ui::Entry *entry = static_cast<ui::Entry *>(data);
  std::string filename = ui::open_file(*g_mainwin);
  if (filename.empty()) {
    entry->set_text("(cancelled)");
    return;
  }
  entry->set_text(filename.c_str());
}

static void on_open_folder_clicked(uiButton *sender, void *data) {
  (void)sender;
  ui::Entry *entry = static_cast<ui::Entry *>(data);
  std::string filename = ui::open_folder(*g_mainwin);
  if (filename.empty()) {
    entry->set_text("(cancelled)");
    return;
  }
  entry->set_text(filename.c_str());
}

static void on_save_file_clicked(uiButton *sender, void *data) {
  (void)sender;
  ui::Entry *entry = static_cast<ui::Entry *>(data);
  std::string filename = ui::save_file(*g_mainwin);
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
  g_open_file_entry = ui::Entry::New().readonly(true);
  g_open_file_button =
      ui::Button::New("  Open File  ").on_clicked(on_open_file_clicked, &g_open_file_entry);
  g_open_folder_entry = ui::Entry::New().readonly(true);
  g_open_folder_button =
      ui::Button::New("Open Folder").on_clicked(on_open_folder_clicked, &g_open_folder_entry);
  g_save_file_entry = ui::Entry::New().readonly(true);
  g_save_file_button =
      ui::Button::New("  Save File  ").on_clicked(on_save_file_clicked, &g_save_file_entry);
  g_msg_box_button = ui::Button::New("Message Box").on_clicked(on_msg_box_clicked, nullptr);
  g_error_box_button = ui::Button::New("Error Box").on_clicked(on_msg_box_error_clicked, nullptr);

  return ui::HorizontalBox::New()
      .padded(true)
      .append(ui::VerticalBox::New(
          ui::DateTimePicker::NewDate(),
          ui::DateTimePicker::NewTime(),
          ui::DateTimePicker::New(),
          ui::FontButton::New(),
          ui::ColorButton::New())
                  .padded(true))
      .append(ui::Separator::NewVertical())
      .append(
          ui::VerticalBox::New(
              ui::Grid::New()
                  .padded(true)
                  .append(g_open_file_button, 0, 0, 1, 1, false, uiAlignFill, false, uiAlignFill)
                  .append(g_open_file_entry, 1, 0, 1, 1, true, uiAlignFill, false, uiAlignFill)
                  .append(g_open_folder_button, 0, 1, 1, 1, false, uiAlignFill, false, uiAlignFill)
                  .append(g_open_folder_entry, 1, 1, 1, 1, true, uiAlignFill, false, uiAlignFill)
                  .append(g_save_file_button, 0, 2, 1, 1, false, uiAlignFill, false, uiAlignFill)
                  .append(g_save_file_entry, 1, 2, 1, 1, true, uiAlignFill, false, uiAlignFill)
                  .append(ui::Grid::New()
                              .padded(true)
                              .append(g_msg_box_button, 0, 0, 1, 1, false, uiAlignFill, false,
                                      uiAlignFill)
                              .append(g_error_box_button, 1, 0, 1, 1, false, uiAlignFill, false,
                                      uiAlignFill),
                          0, 3, 2, 1, false, uiAlignCenter, false, uiAlignStart))
              .padded(true),
          true);
}

int main() {
  std::string err;
  if (!ui::Application::Init(&err)) {
    fprintf(stderr, "error initializing libui: %s\n", err.c_str());
    return 1;
  }

  ui::Application app;
  ui::Window mainwin = ui::Window::New("libui Control Gallery", 640, 480, true);
  g_mainwin = &mainwin;

  AppContext ctx = {&app, &mainwin};
  mainwin.on_closing(on_closing, &ctx);
  app.on_should_quit(on_should_quit, &ctx);

  ui::Tab tab = ui::Tab::New();
  mainwin.set_child(tab);
  mainwin.margined(true);

  tab.append("Basic Controls", make_basic_controls_page());
  tab.set_margined(0, true);
  tab.append("Numbers and Lists", make_numbers_page());
  tab.set_margined(1, true);
  tab.append("Data Choosers", make_data_choosers_page());
  tab.set_margined(2, true);

  mainwin.show();
  app.run();
  return 0;
}
