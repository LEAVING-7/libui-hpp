#pragma once

#include <cstddef>
#include <cstring>
#include <ctime>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <ui.h>

namespace ui {

class Text;

namespace detail {
Text adopt_text(char *text);
}  // namespace detail

class Text {
 public:
  Text(Text &&other) noexcept : text_(other.text_) { other.text_ = nullptr; }

  Text &operator=(Text &&other) noexcept {
    if (this != &other) {
      reset();
      text_ = other.text_;
      other.text_ = nullptr;
    }
    return *this;
  }

  ~Text() { reset(); }

  Text(const Text &) = delete;
  Text &operator=(const Text &) = delete;

  const char *c_str() const { return text_ != nullptr ? text_ : ""; }

  bool empty() const { return text_ == nullptr || text_[0] == '\0'; }

  std::string str() const { return text_ != nullptr ? std::string(text_) : std::string(); }

 private:
  explicit Text(char *text) : text_(text) {}

  void reset() {
    if (text_ != nullptr) {
      uiFreeText(text_);
      text_ = nullptr;
    }
  }

  friend Text detail::adopt_text(char *text);

  char *text_ = nullptr;
};

namespace detail {
inline Text adopt_text(char *text) { return Text(text); }
}  // namespace detail

constexpr int kTableModelColumnNeverEditable = uiTableModelColumnNeverEditable;
constexpr int kTableModelColumnAlwaysEditable = uiTableModelColumnAlwaysEditable;

template <typename D, typename R>
struct Widget {
  R *w = nullptr;

  uiControl *ctrl() const { return uiControl(w); }

  R *raw() const { return w; }

  void show() { uiControlShow(ctrl()); }

  void hide() { uiControlHide(ctrl()); }

  bool visible() const { return uiControlVisible(ctrl()) != 0; }

  void enable() { uiControlEnable(ctrl()); }

  void disable() { uiControlDisable(ctrl()); }

  bool enabled() const { return uiControlEnabled(ctrl()) != 0; }

  void destroy() {
    if (w != nullptr) {
      uiControlDestroy(ctrl());
      w = nullptr;
    }
  }

  D copy(D &out) {
    out = static_cast<D &>(*this);
    return static_cast<D &>(*this);
  }

  template <typename... Args>
  static D make_into(D &out, Args &&...args) {
    return D::make(std::forward<Args>(args)...).copy(out);
  }

 protected:
  static D from(R *raw) {
    D result;
    result.w = raw;
    return result;
  }
};

template <typename D>
struct BoxBase : Widget<D, uiBox> {
  bool padded() const { return uiBoxPadded(this->w) != 0; }

  D copy() const {
    D result;
    result.w = this->w;
    return result;
  }

  D padded(bool value) {
    uiBoxSetPadded(this->w, value ? 1 : 0);
    return copy();
  }

  int num_children() const { return uiBoxNumChildren(this->w); }

  D delete_at(int index) {
    uiBoxDelete(this->w, index);
    return copy();
  }

  template <typename W>
  D append(W &child, bool stretchy = false) {
    uiBoxAppend(this->w, child.ctrl(), stretchy ? 1 : 0);
    return copy();
  }

  template <typename W>
  D append(W &&child, bool stretchy = false) {
    uiBoxAppend(this->w, std::forward<W>(child).ctrl(), stretchy ? 1 : 0);
    return copy();
  }

  template <typename W>
  D append_stretchy(W &child) {
    uiBoxAppend(this->w, child.ctrl(), 1);
    return copy();
  }

  template <typename W>
  D append_stretchy(W &&child) {
    uiBoxAppend(this->w, std::forward<W>(child).ctrl(), 1);
    return copy();
  }
};

struct Application {
  Application() = delete;

  static bool init(std::string *err) {
    uiInitOptions options{};
    const char *init_err = uiInit(&options);
    if (init_err != nullptr) {
      if (err != nullptr) {
        *err = init_err;
      }
      uiFreeInitError(init_err);
      return false;
    }
    return true;
  }

  static void uninit() { uiUninit(); }

  static void run() { uiMain(); }

  static void quit() { uiQuit(); }

  static void queue_main(void (*fn)(void *data), void *data) { uiQueueMain(fn, data); }

  static void timer(int milliseconds, int (*fn)(void *data), void *data) {
    uiTimer(milliseconds, fn, data);
  }

  static int main_step(int wait) { return uiMainStep(wait); }

  static void main_steps() { uiMainSteps(); }

  static void on_should_quit(int (*fn)(void *data), void *data) { uiOnShouldQuit(fn, data); }
};

struct Window : Widget<Window, uiWindow> {
  static Window make(const char *title, int width, int height, bool has_menubar) {
    return from(uiNewWindow(title, width, height, has_menubar ? 1 : 0));
  }

  static Window wrap(uiWindow *raw) { return from(raw); }

  Text title() const { return detail::adopt_text(uiWindowTitle(w)); }

  Window set_title(const char *title) {
    uiWindowSetTitle(w, title);
    return *this;
  }

  void position(int *x, int *y) const { uiWindowPosition(w, x, y); }

  Window set_position(int x, int y) {
    uiWindowSetPosition(w, x, y);
    return *this;
  }

  void content_size(int *width, int *height) const { uiWindowContentSize(w, width, height); }

  Window set_content_size(int width, int height) {
    uiWindowSetContentSize(w, width, height);
    return *this;
  }

  bool fullscreen() const { return uiWindowFullscreen(w) != 0; }

  Window fullscreen(bool value) {
    uiWindowSetFullscreen(w, value ? 1 : 0);
    return *this;
  }

  bool focused() const { return uiWindowFocused(w) != 0; }

  bool borderless() const { return uiWindowBorderless(w) != 0; }

  Window borderless(bool value) {
    uiWindowSetBorderless(w, value ? 1 : 0);
    return *this;
  }

  bool margined() const { return uiWindowMargined(w) != 0; }

  Window margined(bool value) {
    uiWindowSetMargined(w, value ? 1 : 0);
    return *this;
  }

  bool resizable() const { return uiWindowResizeable(w) != 0; }

  Window resizable(bool value) {
    uiWindowSetResizeable(w, value ? 1 : 0);
    return *this;
  }

  template <typename W>
  Window set_child(W &child) {
    uiWindowSetChild(w, child.ctrl());
    return *this;
  }

  template <typename W>
  Window set_child(W &&child) {
    uiWindowSetChild(w, std::forward<W>(child).ctrl());
    return *this;
  }

  Window on_closing(int (*fn)(uiWindow *sender, void *data), void *data) {
    uiWindowOnClosing(w, fn, data);
    return *this;
  }

  Window on_position_changed(void (*fn)(uiWindow *sender, void *data), void *data) {
    uiWindowOnPositionChanged(w, fn, data);
    return *this;
  }

  Window on_content_size_changed(void (*fn)(uiWindow *sender, void *data), void *data) {
    uiWindowOnContentSizeChanged(w, fn, data);
    return *this;
  }

  Window on_focus_changed(void (*fn)(uiWindow *sender, void *data), void *data) {
    uiWindowOnFocusChanged(w, fn, data);
    return *this;
  }
};

struct VerticalBox : BoxBase<VerticalBox> {
  static VerticalBox make() { return from(uiNewVerticalBox()); }

  template <typename... W, typename std::enable_if_t<
                               (sizeof...(W) > 0)
                                   && ((sizeof...(W) > 1)
                                       || (!std::is_same_v<std::decay_t<W>, VerticalBox> && ...)),
                               int> = 0>
  static VerticalBox make(W &&...children) {
    VerticalBox result = make();
    (result.append(std::forward<W>(children)), ...);
    return result;
  }
};

struct HorizontalBox : BoxBase<HorizontalBox> {
  static HorizontalBox make() { return from(uiNewHorizontalBox()); }

  template <typename... W, typename std::enable_if_t<
                               (sizeof...(W) > 0)
                                   && ((sizeof...(W) > 1)
                                       || (!std::is_same_v<std::decay_t<W>, HorizontalBox> && ...)),
                               int> = 0>
  static HorizontalBox make(W &&...children) {
    HorizontalBox result = make();
    (result.append(std::forward<W>(children)), ...);
    return result;
  }
};

struct Button : Widget<Button, uiButton> {
  static Button make(const char *text) { return from(uiNewButton(text)); }

  Text text() const { return detail::adopt_text(uiButtonText(w)); }

  Button set_text(const char *text) {
    uiButtonSetText(w, text);
    return *this;
  }

  Button on_clicked(void (*fn)(uiButton *sender, void *data), void *data) {
    uiButtonOnClicked(w, fn, data);
    return *this;
  }
};

struct Checkbox : Widget<Checkbox, uiCheckbox> {
  static Checkbox make(const char *text) { return from(uiNewCheckbox(text)); }

  Text text() const { return detail::adopt_text(uiCheckboxText(w)); }

  Checkbox set_text(const char *text) {
    uiCheckboxSetText(w, text);
    return *this;
  }

  bool checked() const { return uiCheckboxChecked(w) != 0; }

  Checkbox set_checked(bool value) {
    uiCheckboxSetChecked(w, value ? 1 : 0);
    return *this;
  }

  Checkbox on_toggled(void (*fn)(uiCheckbox *sender, void *data), void *data) {
    uiCheckboxOnToggled(w, fn, data);
    return *this;
  }
};

struct Label : Widget<Label, uiLabel> {
  static Label make(const char *text) { return from(uiNewLabel(text)); }

  Text text() const { return detail::adopt_text(uiLabelText(w)); }

  Label set_text(const char *text) {
    uiLabelSetText(w, text);
    return *this;
  }
};

struct Entry : Widget<Entry, uiEntry> {
  static Entry make() { return from(uiNewEntry()); }

  static Entry make_password() { return from(uiNewPasswordEntry()); }

  static Entry make_search() { return from(uiNewSearchEntry()); }

  Text text() const { return detail::adopt_text(uiEntryText(w)); }

  Entry set_text(const char *text) {
    uiEntrySetText(w, text);
    return *this;
  }

  bool readonly() const { return uiEntryReadOnly(w) != 0; }

  Entry readonly(bool value) {
    uiEntrySetReadOnly(w, value ? 1 : 0);
    return *this;
  }

  Entry on_changed(void (*fn)(uiEntry *sender, void *data), void *data) {
    uiEntryOnChanged(w, fn, data);
    return *this;
  }
};

struct Tab : Widget<Tab, uiTab> {
  static Tab make() { return from(uiNewTab()); }

  int selected() const { return uiTabSelected(w); }

  Tab set_selected(int index) {
    uiTabSetSelected(w, index);
    return *this;
  }

  int num_pages() const { return uiTabNumPages(w); }

  bool margined(int index) const { return uiTabMargined(w, index) != 0; }

  Tab set_margined(int index, bool value) {
    uiTabSetMargined(w, index, value ? 1 : 0);
    return *this;
  }

  template <typename Page>
  Tab append(const char *name, Page &page) {
    uiTabAppend(w, name, page.ctrl());
    return *this;
  }

  template <typename Page>
  Tab append(const char *name, Page &&page) {
    uiTabAppend(w, name, std::forward<Page>(page).ctrl());
    return *this;
  }

  template <typename Page>
  Tab insert_at(const char *name, int index, Page &page) {
    uiTabInsertAt(w, name, index, page.ctrl());
    return *this;
  }

  template <typename Page>
  Tab insert_at(const char *name, int index, Page &&page) {
    uiTabInsertAt(w, name, index, std::forward<Page>(page).ctrl());
    return *this;
  }

  Tab delete_at(int index) {
    uiTabDelete(w, index);
    return *this;
  }

  Tab on_selected(void (*fn)(uiTab *sender, void *data), void *data) {
    uiTabOnSelected(w, fn, data);
    return *this;
  }
};

struct Group : Widget<Group, uiGroup> {
  static Group make(const char *title) { return from(uiNewGroup(title)); }

  Text title() const { return detail::adopt_text(uiGroupTitle(w)); }

  Group set_title(const char *title) {
    uiGroupSetTitle(w, title);
    return *this;
  }

  bool margined() const { return uiGroupMargined(w) != 0; }

  Group margined(bool value) {
    uiGroupSetMargined(w, value ? 1 : 0);
    return *this;
  }

  template <typename W>
  Group set_child(W &child) {
    uiGroupSetChild(w, child.ctrl());
    return *this;
  }

  template <typename W>
  Group set_child(W &&child) {
    uiGroupSetChild(w, std::forward<W>(child).ctrl());
    return *this;
  }
};

struct Spinbox : Widget<Spinbox, uiSpinbox> {
  static Spinbox make(int min, int max) { return from(uiNewSpinbox(min, max)); }

  int value() const { return uiSpinboxValue(w); }

  Spinbox set_value(int value) {
    uiSpinboxSetValue(w, value);
    return *this;
  }

  Spinbox on_changed(void (*fn)(uiSpinbox *sender, void *data), void *data) {
    uiSpinboxOnChanged(w, fn, data);
    return *this;
  }
};

struct Slider : Widget<Slider, uiSlider> {
  static Slider make(int min, int max) { return from(uiNewSlider(min, max)); }

  int value() const { return uiSliderValue(w); }

  Slider set_value(int value) {
    uiSliderSetValue(w, value);
    return *this;
  }

  bool has_tooltip() const { return uiSliderHasToolTip(w) != 0; }

  Slider has_tooltip(bool value) {
    uiSliderSetHasToolTip(w, value ? 1 : 0);
    return *this;
  }

  Slider set_range(int min, int max) {
    uiSliderSetRange(w, min, max);
    return *this;
  }

  Slider on_changed(void (*fn)(uiSlider *sender, void *data), void *data) {
    uiSliderOnChanged(w, fn, data);
    return *this;
  }

  Slider on_released(void (*fn)(uiSlider *sender, void *data), void *data) {
    uiSliderOnReleased(w, fn, data);
    return *this;
  }
};

struct ProgressBar : Widget<ProgressBar, uiProgressBar> {
  static ProgressBar make() { return from(uiNewProgressBar()); }

  int value() const { return uiProgressBarValue(w); }

  ProgressBar set_value(int value) {
    uiProgressBarSetValue(w, value);
    return *this;
  }
};

struct Combobox : Widget<Combobox, uiCombobox> {
  static Combobox make() { return from(uiNewCombobox()); }

  Combobox append(const char *text) {
    uiComboboxAppend(w, text);
    return *this;
  }

  Combobox insert_at(int index, const char *text) {
    uiComboboxInsertAt(w, index, text);
    return *this;
  }

  Combobox delete_at(int index) {
    uiComboboxDelete(w, index);
    return *this;
  }

  Combobox clear() {
    uiComboboxClear(w);
    return *this;
  }

  int num_items() const { return uiComboboxNumItems(w); }

  int selected() const { return uiComboboxSelected(w); }

  Combobox set_selected(int index) {
    uiComboboxSetSelected(w, index);
    return *this;
  }

  Combobox on_selected(void (*fn)(uiCombobox *sender, void *data), void *data) {
    uiComboboxOnSelected(w, fn, data);
    return *this;
  }
};

struct EditableCombobox : Widget<EditableCombobox, uiEditableCombobox> {
  static EditableCombobox make() { return from(uiNewEditableCombobox()); }

  EditableCombobox append(const char *text) {
    uiEditableComboboxAppend(w, text);
    return *this;
  }

  Text text() const { return detail::adopt_text(uiEditableComboboxText(w)); }

  EditableCombobox set_text(const char *text) {
    uiEditableComboboxSetText(w, text);
    return *this;
  }

  EditableCombobox on_changed(void (*fn)(uiEditableCombobox *sender, void *data), void *data) {
    uiEditableComboboxOnChanged(w, fn, data);
    return *this;
  }
};

struct RadioButtons : Widget<RadioButtons, uiRadioButtons> {
  static RadioButtons make() { return from(uiNewRadioButtons()); }

  RadioButtons append(const char *text) {
    uiRadioButtonsAppend(w, text);
    return *this;
  }

  int selected() const { return uiRadioButtonsSelected(w); }

  RadioButtons set_selected(int index) {
    uiRadioButtonsSetSelected(w, index);
    return *this;
  }

  RadioButtons on_selected(void (*fn)(uiRadioButtons *sender, void *data), void *data) {
    uiRadioButtonsOnSelected(w, fn, data);
    return *this;
  }
};

struct DateTimePicker : Widget<DateTimePicker, uiDateTimePicker> {
  static DateTimePicker make() { return from(uiNewDateTimePicker()); }

  static DateTimePicker make_date() { return from(uiNewDatePicker()); }

  static DateTimePicker make_time() { return from(uiNewTimePicker()); }

  void time(struct tm *out) const { uiDateTimePickerTime(w, out); }

  DateTimePicker set_time(const struct tm *time) {
    uiDateTimePickerSetTime(w, time);
    return *this;
  }

  DateTimePicker on_changed(void (*fn)(uiDateTimePicker *sender, void *data), void *data) {
    uiDateTimePickerOnChanged(w, fn, data);
    return *this;
  }
};

struct MultilineEntry : Widget<MultilineEntry, uiMultilineEntry> {
  static MultilineEntry make() { return from(uiNewMultilineEntry()); }

  static MultilineEntry make_non_wrapping() { return from(uiNewNonWrappingMultilineEntry()); }

  Text text() const { return detail::adopt_text(uiMultilineEntryText(w)); }

  MultilineEntry set_text(const char *text) {
    uiMultilineEntrySetText(w, text);
    return *this;
  }

  MultilineEntry append(const char *text) {
    uiMultilineEntryAppend(w, text);
    return *this;
  }

  bool readonly() const { return uiMultilineEntryReadOnly(w) != 0; }

  MultilineEntry readonly(bool value) {
    uiMultilineEntrySetReadOnly(w, value ? 1 : 0);
    return *this;
  }

  MultilineEntry on_changed(void (*fn)(uiMultilineEntry *sender, void *data), void *data) {
    uiMultilineEntryOnChanged(w, fn, data);
    return *this;
  }
};

struct ColorButton : Widget<ColorButton, uiColorButton> {
  static ColorButton make() { return from(uiNewColorButton()); }

  void color(double *r, double *g, double *b, double *a) const {
    uiColorButtonColor(w, r, g, b, a);
  }

  ColorButton set_color(double r, double g, double b, double a) {
    uiColorButtonSetColor(w, r, g, b, a);
    return *this;
  }

  ColorButton on_changed(void (*fn)(uiColorButton *sender, void *data), void *data) {
    uiColorButtonOnChanged(w, fn, data);
    return *this;
  }
};

struct MenuItem {
  uiMenuItem *item = nullptr;

  static MenuItem from(uiMenuItem *raw) {
    MenuItem result;
    result.item = raw;
    return result;
  }

  MenuItem enable() {
    uiMenuItemEnable(item);
    return *this;
  }

  MenuItem disable() {
    uiMenuItemDisable(item);
    return *this;
  }

  bool checked() const { return uiMenuItemChecked(item) != 0; }

  MenuItem set_checked(bool value) {
    uiMenuItemSetChecked(item, value ? 1 : 0);
    return *this;
  }

  MenuItem on_clicked(void (*fn)(uiMenuItem *sender, uiWindow *window, void *data), void *data) {
    uiMenuItemOnClicked(item, fn, data);
    return *this;
  }
};

struct Menu {
  uiMenu *m = nullptr;

  static Menu make(const char *name) {
    Menu result;
    result.m = uiNewMenu(name);
    return result;
  }

  MenuItem append_item(const char *name) { return MenuItem::from(uiMenuAppendItem(m, name)); }

  MenuItem append_check_item(const char *name) {
    return MenuItem::from(uiMenuAppendCheckItem(m, name));
  }

  MenuItem append_quit_item() { return MenuItem::from(uiMenuAppendQuitItem(m)); }

  MenuItem append_preferences_item() { return MenuItem::from(uiMenuAppendPreferencesItem(m)); }

  MenuItem append_about_item() { return MenuItem::from(uiMenuAppendAboutItem(m)); }

  Menu append_separator() {
    uiMenuAppendSeparator(m);
    return *this;
  }

  uiMenu *raw() const { return m; }
};

struct FontDescriptor {
  uiFontDescriptor desc{};

  FontDescriptor() = default;

  ~FontDescriptor() { free(); }

  FontDescriptor(const FontDescriptor &) = delete;
  FontDescriptor &operator=(const FontDescriptor &) = delete;

  FontDescriptor(FontDescriptor &&other) noexcept
      : desc(other.desc), from_button_(other.from_button_) {
    other.desc = {};
    other.from_button_ = false;
  }

  FontDescriptor &operator=(FontDescriptor &&other) noexcept {
    if (this != &other) {
      free();
      desc = other.desc;
      from_button_ = other.from_button_;
      other.desc = {};
      other.from_button_ = false;
    }
    return *this;
  }

  static FontDescriptor load_control_font() {
    FontDescriptor result;
    uiLoadControlFont(&result.desc);
    return result;
  }

 private:
  friend struct FontButton;
  bool from_button_ = false;

  void free() {
    if (from_button_) {
      uiFreeFontButtonFont(&desc);
      from_button_ = false;
    } else if (desc.Family != nullptr) {
      uiFreeFontDescriptor(&desc);
    }
    desc = {};
  }
};

struct FontButton : Widget<FontButton, uiFontButton> {
  static FontButton make() { return from(uiNewFontButton()); }

  FontDescriptor font() const {
    FontDescriptor result;
    uiFontButtonFont(w, &result.desc);
    result.from_button_ = true;
    return result;
  }

  FontButton on_changed(void (*fn)(uiFontButton *sender, void *data), void *data) {
    uiFontButtonOnChanged(w, fn, data);
    return *this;
  }
};

class Image {
 public:
  Image() = default;

  static Image make(double width, double height) {
    Image result;
    result.img_ = uiNewImage(width, height);
    return result;
  }

  ~Image() { reset(); }

  Image(const Image &) = delete;
  Image &operator=(const Image &) = delete;

  Image(Image &&other) noexcept : img_(other.img_) { other.img_ = nullptr; }

  Image &operator=(Image &&other) noexcept {
    if (this != &other) {
      reset();
      img_ = other.img_;
      other.img_ = nullptr;
    }
    return *this;
  }

  Image &append(void *pixels, int pixel_width, int pixel_height, int byte_stride) {
    uiImageAppend(img_, pixels, pixel_width, pixel_height, byte_stride);
    return *this;
  }

  uiImage *raw() const { return img_; }

 private:
  uiImage *img_ = nullptr;

  void reset() {
    if (img_ != nullptr) {
      uiFreeImage(img_);
      img_ = nullptr;
    }
  }
};

class TableValue {
 public:
  TableValue() = default;

  ~TableValue() { reset(); }

  TableValue(const TableValue &) = delete;
  TableValue &operator=(const TableValue &) = delete;

  TableValue(TableValue &&other) noexcept : value_(other.value_) { other.value_ = nullptr; }

  TableValue &operator=(TableValue &&other) noexcept {
    if (this != &other) {
      reset();
      value_ = other.value_;
      other.value_ = nullptr;
    }
    return *this;
  }

  bool empty() const { return value_ == nullptr; }

  uiTableValueType type() const { return uiTableValueGetType(value_); }

  const char *as_string() const { return uiTableValueString(value_); }

  uiImage *as_image() const { return uiTableValueImage(value_); }

  int as_int() const { return uiTableValueInt(value_); }

  void as_color(double *r, double *g, double *b, double *a) const {
    uiTableValueColor(value_, r, g, b, a);
  }

  uiTableValue *release() {
    uiTableValue *result = value_;
    value_ = nullptr;
    return result;
  }

  static TableValue string(const char *str) {
    TableValue result;
    result.value_ = uiNewTableValueString(str);
    return result;
  }

  static TableValue image(Image &img) {
    TableValue result;
    result.value_ = uiNewTableValueImage(img.raw());
    return result;
  }

  static TableValue integer(int i) {
    TableValue result;
    result.value_ = uiNewTableValueInt(i);
    return result;
  }

  static TableValue color(double r, double g, double b, double a) {
    TableValue result;
    result.value_ = uiNewTableValueColor(r, g, b, a);
    return result;
  }

  static TableValue borrow(const uiTableValue *value) {
    TableValue result;
    result.value_ = const_cast<uiTableValue *>(value);
    result.owns_ = false;
    return result;
  }

 private:
  friend class TableModel;
  uiTableValue *value_ = nullptr;
  bool owns_ = true;

  void reset() {
    if (value_ != nullptr && owns_) {
      uiFreeTableValue(value_);
    }
    value_ = nullptr;
    owns_ = true;
  }
};

class TableModel;

struct TableModelHandler {
  virtual ~TableModelHandler() = default;
  virtual void attach(TableModel *model) { (void)model; }
  virtual int num_columns() = 0;
  virtual uiTableValueType column_type(int column) = 0;
  virtual int num_rows() = 0;
  virtual TableValue cell_value(int row, int column) = 0;
  virtual void set_cell_value(int row, int column, const TableValue &value) = 0;
};

class TableModel {
 public:
  TableModel() = default;

  template <typename H, typename... Args>
  static TableModel make(Args &&...args) {
    return TableModel(std::make_unique<H>(std::forward<Args>(args)...));
  }

  static TableModel make(std::unique_ptr<TableModelHandler> handler) {
    return TableModel(std::move(handler));
  }

  TableModel(const TableModel &) = delete;
  TableModel &operator=(const TableModel &) = delete;

  TableModel(TableModel &&) noexcept = default;
  TableModel &operator=(TableModel &&) noexcept = default;

  ~TableModel() = default;

  uiTableModel *raw() const { return impl_ != nullptr ? impl_->model : nullptr; }

  TableModelHandler *handler() { return impl_ != nullptr ? impl_->handler.get() : nullptr; }

  const TableModelHandler *handler() const {
    return impl_ != nullptr ? impl_->handler.get() : nullptr;
  }

  template <typename H>
  H *handler_as() {
    return dynamic_cast<H *>(handler());
  }

  template <typename H>
  const H *handler_as() const {
    return dynamic_cast<const H *>(handler());
  }

  void row_inserted(int index) { uiTableModelRowInserted(impl_->model, index); }

  void row_changed(int index) { uiTableModelRowChanged(impl_->model, index); }

  void row_deleted(int index) { uiTableModelRowDeleted(impl_->model, index); }

 private:
  struct Impl {
    std::unique_ptr<TableModelHandler> handler;
    uiTableModelHandler c_handler{};
    uiTableModel *model = nullptr;

    explicit Impl(std::unique_ptr<TableModelHandler> handler_in)
        : handler(std::move(handler_in)) {
      c_handler.NumColumns = &tramp_num_columns;
      c_handler.ColumnType = &tramp_column_type;
      c_handler.NumRows = &tramp_num_rows;
      c_handler.CellValue = &tramp_cell_value;
      c_handler.SetCellValue = &tramp_set_cell_value;
      model = uiNewTableModel(&c_handler);
    }

    ~Impl() {
      if (model != nullptr) {
        uiFreeTableModel(model);
      }
    }

    Impl(const Impl &) = delete;
    Impl &operator=(const Impl &) = delete;

    static Impl *self(uiTableModelHandler *mh) {
      return reinterpret_cast<Impl *>(reinterpret_cast<char *>(mh) - offsetof(Impl, c_handler));
    }

    static int tramp_num_columns(uiTableModelHandler *mh, uiTableModel *) {
      return self(mh)->handler->num_columns();
    }

    static uiTableValueType tramp_column_type(uiTableModelHandler *mh, uiTableModel *, int column) {
      return self(mh)->handler->column_type(column);
    }

    static int tramp_num_rows(uiTableModelHandler *mh, uiTableModel *) {
      return self(mh)->handler->num_rows();
    }

    static uiTableValue *tramp_cell_value(uiTableModelHandler *mh, uiTableModel *, int row,
                                          int column) {
      TableValue value = self(mh)->handler->cell_value(row, column);
      if (value.empty()) {
        return nullptr;
      }
      return value.release();
    }

    static void tramp_set_cell_value(uiTableModelHandler *mh, uiTableModel *, int row, int column,
                                     const uiTableValue *value) {
      if (value == nullptr) {
        self(mh)->handler->set_cell_value(row, column, TableValue());
        return;
      }
      self(mh)->handler->set_cell_value(row, column, TableValue::borrow(value));
    }
  };

  explicit TableModel(std::unique_ptr<TableModelHandler> handler)
      : impl_(std::make_unique<Impl>(std::move(handler))) {
    impl_->handler->attach(this);
  }

  std::unique_ptr<Impl> impl_;
};

class TableSelection {
 public:
  TableSelection() = default;

  explicit TableSelection(uiTableSelection *raw) : selection_(raw) {}

  ~TableSelection() { reset(); }

  TableSelection(const TableSelection &) = delete;
  TableSelection &operator=(const TableSelection &) = delete;

  TableSelection(TableSelection &&other) noexcept : selection_(other.selection_) {
    other.selection_ = nullptr;
  }

  TableSelection &operator=(TableSelection &&other) noexcept {
    if (this != &other) {
      reset();
      selection_ = other.selection_;
      other.selection_ = nullptr;
    }
    return *this;
  }

  int num_rows() const { return selection_ != nullptr ? selection_->NumRows : 0; }

  int row(int index) const { return selection_->Rows[index]; }

  std::vector<int> rows() const {
    std::vector<int> result;
    if (selection_ == nullptr) {
      return result;
    }
    result.reserve(static_cast<size_t>(selection_->NumRows));
    for (int i = 0; i < selection_->NumRows; ++i) {
      result.push_back(selection_->Rows[i]);
    }
    return result;
  }

 private:
  friend struct Table;
  uiTableSelection *selection_ = nullptr;

  void reset() {
    if (selection_ != nullptr) {
      uiFreeTableSelection(selection_);
      selection_ = nullptr;
    }
  }
};

struct TableTextColumnParams {
  int color_model_column = -1;
};

struct Table : Widget<Table, uiTable> {
  struct Params {
    TableModel *model = nullptr;
    int row_background_color_model_column = -1;
  };

  static Table make(Params &params) {
    uiTableParams p{};
    p.Model = params.model->raw();
    p.RowBackgroundColorModelColumn = params.row_background_color_model_column;
    return from(uiNewTable(&p));
  }

  Table append_text_column(const char *name, int text_model_column, int text_editable_model_column,
                           const TableTextColumnParams *text_params = nullptr) {
    uiTableTextColumnOptionalParams tp{};
    uiTableTextColumnOptionalParams *tp_ptr = nullptr;
    if (text_params != nullptr) {
      tp.ColorModelColumn = text_params->color_model_column;
      tp_ptr = &tp;
    }
    uiTableAppendTextColumn(w, name, text_model_column, text_editable_model_column, tp_ptr);
    return *this;
  }

  Table append_image_column(const char *name, int image_model_column) {
    uiTableAppendImageColumn(w, name, image_model_column);
    return *this;
  }

  Table append_image_text_column(const char *name, int image_model_column, int text_model_column,
                                 int text_editable_model_column,
                                 const TableTextColumnParams *text_params = nullptr) {
    uiTableTextColumnOptionalParams tp{};
    uiTableTextColumnOptionalParams *tp_ptr = nullptr;
    if (text_params != nullptr) {
      tp.ColorModelColumn = text_params->color_model_column;
      tp_ptr = &tp;
    }
    uiTableAppendImageTextColumn(w, name, image_model_column, text_model_column,
                                 text_editable_model_column, tp_ptr);
    return *this;
  }

  Table append_checkbox_column(const char *name, int checkbox_model_column,
                               int checkbox_editable_model_column) {
    uiTableAppendCheckboxColumn(w, name, checkbox_model_column, checkbox_editable_model_column);
    return *this;
  }

  Table append_checkbox_text_column(const char *name, int checkbox_model_column,
                                    int checkbox_editable_model_column, int text_model_column,
                                    int text_editable_model_column,
                                    const TableTextColumnParams *text_params = nullptr) {
    uiTableTextColumnOptionalParams tp{};
    uiTableTextColumnOptionalParams *tp_ptr = nullptr;
    if (text_params != nullptr) {
      tp.ColorModelColumn = text_params->color_model_column;
      tp_ptr = &tp;
    }
    uiTableAppendCheckboxTextColumn(w, name, checkbox_model_column, checkbox_editable_model_column,
                                    text_model_column, text_editable_model_column, tp_ptr);
    return *this;
  }

  Table append_progress_bar_column(const char *name, int progress_model_column) {
    uiTableAppendProgressBarColumn(w, name, progress_model_column);
    return *this;
  }

  Table append_button_column(const char *name, int button_model_column,
                             int button_clickable_model_column) {
    uiTableAppendButtonColumn(w, name, button_model_column, button_clickable_model_column);
    return *this;
  }

  bool header_visible() const { return uiTableHeaderVisible(w) != 0; }

  Table header_visible(bool value) {
    uiTableHeaderSetVisible(w, value ? 1 : 0);
    return *this;
  }

  Table header_sort_indicator(int column, uiSortIndicator indicator) {
    uiTableHeaderSetSortIndicator(w, column, indicator);
    return *this;
  }

  uiSortIndicator header_sort_indicator(int column) const {
    return uiTableHeaderSortIndicator(w, column);
  }

  int column_width(int column) const { return uiTableColumnWidth(w, column); }

  Table set_column_width(int column, int width) {
    uiTableColumnSetWidth(w, column, width);
    return *this;
  }

  uiTableSelectionMode selection_mode() const { return uiTableGetSelectionMode(w); }

  Table set_selection_mode(uiTableSelectionMode mode) {
    uiTableSetSelectionMode(w, mode);
    return *this;
  }

  TableSelection selection() const { return TableSelection(uiTableGetSelection(w)); }

  Table set_selection(const std::vector<int> &rows) {
    uiTableSelection sel{};
    sel.NumRows = static_cast<int>(rows.size());
    sel.Rows = const_cast<int *>(rows.data());
    uiTableSetSelection(w, &sel);
    return *this;
  }

  Table on_row_clicked(void (*fn)(uiTable *sender, int row, void *data), void *data) {
    uiTableOnRowClicked(w, fn, data);
    return *this;
  }

  Table on_row_double_clicked(void (*fn)(uiTable *sender, int row, void *data), void *data) {
    uiTableOnRowDoubleClicked(w, fn, data);
    return *this;
  }

  Table on_selection_changed(void (*fn)(uiTable *sender, void *data), void *data) {
    uiTableOnSelectionChanged(w, fn, data);
    return *this;
  }

  Table header_on_clicked(void (*fn)(uiTable *sender, int column, void *data), void *data) {
    uiTableHeaderOnClicked(w, fn, data);
    return *this;
  }
};

struct Separator : Widget<Separator, uiSeparator> {
  static Separator make_horizontal() { return from(uiNewHorizontalSeparator()); }

  static Separator make_vertical() { return from(uiNewVerticalSeparator()); }
};

struct Form : Widget<Form, uiForm> {
  static Form make() { return from(uiNewForm()); }

  bool padded() const { return uiFormPadded(w) != 0; }

  Form padded(bool value) {
    uiFormSetPadded(w, value ? 1 : 0);
    return *this;
  }

  int num_children() const { return uiFormNumChildren(w); }

  Form delete_at(int index) {
    uiFormDelete(w, index);
    return *this;
  }

  template <typename W>
  Form append(const char *label, W &child, bool stretchy = false) {
    uiFormAppend(w, label, child.ctrl(), stretchy ? 1 : 0);
    return *this;
  }

  template <typename W>
  Form append(const char *label, W &&child, bool stretchy = false) {
    uiFormAppend(w, label, std::forward<W>(child).ctrl(), stretchy ? 1 : 0);
    return *this;
  }
};

struct Grid : Widget<Grid, uiGrid> {
  static Grid make() { return from(uiNewGrid()); }

  bool padded() const { return uiGridPadded(w) != 0; }

  Grid padded(bool value) {
    uiGridSetPadded(w, value ? 1 : 0);
    return *this;
  }

  template <typename W>
  Grid append(W &child, int left, int top, int xspan, int yspan, bool hexpand, uiAlign halign,
              bool vexpand, uiAlign valign) {
    uiGridAppend(w, child.ctrl(), left, top, xspan, yspan, hexpand ? 1 : 0, halign, vexpand ? 1 : 0,
                 valign);
    return *this;
  }

  template <typename W>
  Grid append(W &&child, int left, int top, int xspan, int yspan, bool hexpand, uiAlign halign,
              bool vexpand, uiAlign valign) {
    uiGridAppend(w, std::forward<W>(child).ctrl(), left, top, xspan, yspan, hexpand ? 1 : 0, halign,
                 vexpand ? 1 : 0, valign);
    return *this;
  }

  template <typename W>
  Grid insert_at(W &child, uiControl *existing, uiAt at, int xspan, int yspan, bool hexpand,
                 uiAlign halign, bool vexpand, uiAlign valign) {
    uiGridInsertAt(w, child.ctrl(), existing, at, xspan, yspan, hexpand ? 1 : 0, halign,
                   vexpand ? 1 : 0, valign);
    return *this;
  }

  template <typename W>
  Grid insert_at(W &&child, uiControl *existing, uiAt at, int xspan, int yspan, bool hexpand,
                 uiAlign halign, bool vexpand, uiAlign valign) {
    uiGridInsertAt(w, std::forward<W>(child).ctrl(), existing, at, xspan, yspan, hexpand ? 1 : 0,
                   halign, vexpand ? 1 : 0, valign);
    return *this;
  }

  template <typename Child, typename Existing>
  Grid insert_at(Child &&child, Existing &existing, uiAt at, int xspan, int yspan, bool hexpand,
                 uiAlign halign, bool vexpand, uiAlign valign) {
    return insert_at(std::forward<Child>(child), existing.ctrl(), at, xspan, yspan, hexpand, halign,
                     vexpand, valign);
  }
};

inline Text open_file(Window &parent) { return detail::adopt_text(uiOpenFile(parent.raw())); }

inline Text open_folder(Window &parent) { return detail::adopt_text(uiOpenFolder(parent.raw())); }

inline Text save_file(Window &parent) { return detail::adopt_text(uiSaveFile(parent.raw())); }

inline void msg_box(Window &parent, const char *title, const char *description) {
  uiMsgBox(parent.raw(), title, description);
}

inline void msg_box_error(Window &parent, const char *title, const char *description) {
  uiMsgBoxError(parent.raw(), title, description);
}

}  // namespace ui
