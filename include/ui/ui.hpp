#pragma once

#include <cstddef>
#include <cstring>
#include <ctime>
#include <initializer_list>
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
  uiControl *ctrl() const { return uiControl(w_); }

  R *raw() const { return w_; }

  void show() { uiControlShow(ctrl()); }

  void hide() { uiControlHide(ctrl()); }

  bool visible() const { return uiControlVisible(ctrl()) != 0; }

  void enable() { uiControlEnable(ctrl()); }

  void disable() { uiControlDisable(ctrl()); }

  bool enabled() const { return uiControlEnabled(ctrl()) != 0; }

  void destroy() {
    if (w_ != nullptr) {
      uiControlDestroy(ctrl());
      w_ = nullptr;
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

  static D wrap(R *raw) {
    D result;
    result.w_ = raw;
    return result;
  }

 protected:
  R *w_ = nullptr;
};

template <typename D>
struct BoxBase : Widget<D, uiBox> {
  bool padded() const { return uiBoxPadded(this->w_) != 0; }

  D copy() const {
    D result;
    result.w_ = this->w_;
    return result;
  }

  D padded(bool value) {
    uiBoxSetPadded(this->w_, value ? 1 : 0);
    return copy();
  }

  int num_children() const { return uiBoxNumChildren(this->w_); }

  D delete_at(int index) {
    uiBoxDelete(this->w_, index);
    return copy();
  }

  template <typename W>
  D append(W &child, bool stretchy = false) {
    uiBoxAppend(this->w_, child.ctrl(), stretchy ? 1 : 0);
    return copy();
  }

  template <typename W>
  D append(W &&child, bool stretchy = false) {
    uiBoxAppend(this->w_, std::forward<W>(child).ctrl(), stretchy ? 1 : 0);
    return copy();
  }

  template <typename W>
  D append_stretchy(W &child) {
    uiBoxAppend(this->w_, child.ctrl(), 1);
    return copy();
  }

  template <typename W>
  D append_stretchy(W &&child) {
    uiBoxAppend(this->w_, std::forward<W>(child).ctrl(), 1);
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
    return wrap(uiNewWindow(title, width, height, has_menubar ? 1 : 0));
  }

  Text title() const { return detail::adopt_text(uiWindowTitle(w_)); }

  Window set_title(const char *title) {
    uiWindowSetTitle(w_, title);
    return *this;
  }

  void position(int *x, int *y) const { uiWindowPosition(w_, x, y); }

  Window set_position(int x, int y) {
    uiWindowSetPosition(w_, x, y);
    return *this;
  }

  void content_size(int *width, int *height) const { uiWindowContentSize(w_, width, height); }

  Window set_content_size(int width, int height) {
    uiWindowSetContentSize(w_, width, height);
    return *this;
  }

  bool fullscreen() const { return uiWindowFullscreen(w_) != 0; }

  Window fullscreen(bool value) {
    uiWindowSetFullscreen(w_, value ? 1 : 0);
    return *this;
  }

  bool focused() const { return uiWindowFocused(w_) != 0; }

  bool borderless() const { return uiWindowBorderless(w_) != 0; }

  Window borderless(bool value) {
    uiWindowSetBorderless(w_, value ? 1 : 0);
    return *this;
  }

  bool margined() const { return uiWindowMargined(w_) != 0; }

  Window margined(bool value) {
    uiWindowSetMargined(w_, value ? 1 : 0);
    return *this;
  }

  bool resizable() const { return uiWindowResizeable(w_) != 0; }

  Window resizable(bool value) {
    uiWindowSetResizeable(w_, value ? 1 : 0);
    return *this;
  }

  template <typename W>
  Window set_child(W &child) {
    uiWindowSetChild(w_, child.ctrl());
    return *this;
  }

  template <typename W>
  Window set_child(W &&child) {
    uiWindowSetChild(w_, std::forward<W>(child).ctrl());
    return *this;
  }

  Window on_closing(int (*fn)(uiWindow *sender, void *data), void *data) {
    uiWindowOnClosing(w_, fn, data);
    return *this;
  }

  Window on_position_changed(void (*fn)(uiWindow *sender, void *data), void *data) {
    uiWindowOnPositionChanged(w_, fn, data);
    return *this;
  }

  Window on_content_size_changed(void (*fn)(uiWindow *sender, void *data), void *data) {
    uiWindowOnContentSizeChanged(w_, fn, data);
    return *this;
  }

  Window on_focus_changed(void (*fn)(uiWindow *sender, void *data), void *data) {
    uiWindowOnFocusChanged(w_, fn, data);
    return *this;
  }
};

struct VerticalBox : BoxBase<VerticalBox> {
  static VerticalBox make() { return wrap(uiNewVerticalBox()); }

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
  static HorizontalBox make() { return wrap(uiNewHorizontalBox()); }

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
  static Button make(const char *text) { return wrap(uiNewButton(text)); }

  Text text() const { return detail::adopt_text(uiButtonText(w_)); }

  Button set_text(const char *text) {
    uiButtonSetText(w_, text);
    return *this;
  }

  Button on_clicked(void (*fn)(uiButton *sender, void *data), void *data) {
    uiButtonOnClicked(w_, fn, data);
    return *this;
  }
};

struct Checkbox : Widget<Checkbox, uiCheckbox> {
  static Checkbox make(const char *text) { return wrap(uiNewCheckbox(text)); }

  Text text() const { return detail::adopt_text(uiCheckboxText(w_)); }

  Checkbox set_text(const char *text) {
    uiCheckboxSetText(w_, text);
    return *this;
  }

  bool checked() const { return uiCheckboxChecked(w_) != 0; }

  Checkbox set_checked(bool value) {
    uiCheckboxSetChecked(w_, value ? 1 : 0);
    return *this;
  }

  Checkbox on_toggled(void (*fn)(uiCheckbox *sender, void *data), void *data) {
    uiCheckboxOnToggled(w_, fn, data);
    return *this;
  }
};

struct Label : Widget<Label, uiLabel> {
  static Label make(const char *text) { return wrap(uiNewLabel(text)); }

  Text text() const { return detail::adopt_text(uiLabelText(w_)); }

  Label set_text(const char *text) {
    uiLabelSetText(w_, text);
    return *this;
  }
};

struct Entry : Widget<Entry, uiEntry> {
  static Entry make() { return wrap(uiNewEntry()); }

  static Entry make_password() { return wrap(uiNewPasswordEntry()); }

  static Entry make_search() { return wrap(uiNewSearchEntry()); }

  Text text() const { return detail::adopt_text(uiEntryText(w_)); }

  Entry set_text(const char *text) {
    uiEntrySetText(w_, text);
    return *this;
  }

  bool readonly() const { return uiEntryReadOnly(w_) != 0; }

  Entry readonly(bool value) {
    uiEntrySetReadOnly(w_, value ? 1 : 0);
    return *this;
  }

  Entry on_changed(void (*fn)(uiEntry *sender, void *data), void *data) {
    uiEntryOnChanged(w_, fn, data);
    return *this;
  }
};

struct Tab : Widget<Tab, uiTab> {
  static Tab make() { return wrap(uiNewTab()); }

  int selected() const { return uiTabSelected(w_); }

  Tab set_selected(int index) {
    uiTabSetSelected(w_, index);
    return *this;
  }

  int num_pages() const { return uiTabNumPages(w_); }

  bool margined(int index) const { return uiTabMargined(w_, index) != 0; }

  Tab set_margined(int index, bool value) {
    uiTabSetMargined(w_, index, value ? 1 : 0);
    return *this;
  }

  template <typename Page>
  Tab append(const char *name, Page &page) {
    uiTabAppend(w_, name, page.ctrl());
    return *this;
  }

  template <typename Page>
  Tab append(const char *name, Page &&page) {
    uiTabAppend(w_, name, std::forward<Page>(page).ctrl());
    return *this;
  }

  template <typename Page>
  Tab insert_at(const char *name, int index, Page &page) {
    uiTabInsertAt(w_, name, index, page.ctrl());
    return *this;
  }

  template <typename Page>
  Tab insert_at(const char *name, int index, Page &&page) {
    uiTabInsertAt(w_, name, index, std::forward<Page>(page).ctrl());
    return *this;
  }

  Tab delete_at(int index) {
    uiTabDelete(w_, index);
    return *this;
  }

  Tab on_selected(void (*fn)(uiTab *sender, void *data), void *data) {
    uiTabOnSelected(w_, fn, data);
    return *this;
  }
};

struct Group : Widget<Group, uiGroup> {
  static Group make(const char *title) { return wrap(uiNewGroup(title)); }

  Text title() const { return detail::adopt_text(uiGroupTitle(w_)); }

  Group set_title(const char *title) {
    uiGroupSetTitle(w_, title);
    return *this;
  }

  bool margined() const { return uiGroupMargined(w_) != 0; }

  Group margined(bool value) {
    uiGroupSetMargined(w_, value ? 1 : 0);
    return *this;
  }

  template <typename W>
  Group set_child(W &child) {
    uiGroupSetChild(w_, child.ctrl());
    return *this;
  }

  template <typename W>
  Group set_child(W &&child) {
    uiGroupSetChild(w_, std::forward<W>(child).ctrl());
    return *this;
  }
};

struct Spinbox : Widget<Spinbox, uiSpinbox> {
  static Spinbox make(int min, int max) { return wrap(uiNewSpinbox(min, max)); }

  int value() const { return uiSpinboxValue(w_); }

  Spinbox set_value(int value) {
    uiSpinboxSetValue(w_, value);
    return *this;
  }

  Spinbox on_changed(void (*fn)(uiSpinbox *sender, void *data), void *data) {
    uiSpinboxOnChanged(w_, fn, data);
    return *this;
  }
};

struct Slider : Widget<Slider, uiSlider> {
  static Slider make(int min, int max) { return wrap(uiNewSlider(min, max)); }

  int value() const { return uiSliderValue(w_); }

  Slider set_value(int value) {
    uiSliderSetValue(w_, value);
    return *this;
  }

  bool has_tooltip() const { return uiSliderHasToolTip(w_) != 0; }

  Slider has_tooltip(bool value) {
    uiSliderSetHasToolTip(w_, value ? 1 : 0);
    return *this;
  }

  Slider set_range(int min, int max) {
    uiSliderSetRange(w_, min, max);
    return *this;
  }

  Slider on_changed(void (*fn)(uiSlider *sender, void *data), void *data) {
    uiSliderOnChanged(w_, fn, data);
    return *this;
  }

  Slider on_released(void (*fn)(uiSlider *sender, void *data), void *data) {
    uiSliderOnReleased(w_, fn, data);
    return *this;
  }
};

struct ProgressBar : Widget<ProgressBar, uiProgressBar> {
  static ProgressBar make() { return wrap(uiNewProgressBar()); }

  int value() const { return uiProgressBarValue(w_); }

  ProgressBar set_value(int value) {
    uiProgressBarSetValue(w_, value);
    return *this;
  }
};

struct Combobox : Widget<Combobox, uiCombobox> {
  static Combobox make() { return wrap(uiNewCombobox()); }

  Combobox append(const char *text) {
    uiComboboxAppend(w_, text);
    return *this;
  }

  Combobox insert_at(int index, const char *text) {
    uiComboboxInsertAt(w_, index, text);
    return *this;
  }

  Combobox delete_at(int index) {
    uiComboboxDelete(w_, index);
    return *this;
  }

  Combobox clear() {
    uiComboboxClear(w_);
    return *this;
  }

  int num_items() const { return uiComboboxNumItems(w_); }

  int selected() const { return uiComboboxSelected(w_); }

  Combobox set_selected(int index) {
    uiComboboxSetSelected(w_, index);
    return *this;
  }

  Combobox on_selected(void (*fn)(uiCombobox *sender, void *data), void *data) {
    uiComboboxOnSelected(w_, fn, data);
    return *this;
  }
};

struct EditableCombobox : Widget<EditableCombobox, uiEditableCombobox> {
  static EditableCombobox make() { return wrap(uiNewEditableCombobox()); }

  EditableCombobox append(const char *text) {
    uiEditableComboboxAppend(w_, text);
    return *this;
  }

  Text text() const { return detail::adopt_text(uiEditableComboboxText(w_)); }

  EditableCombobox set_text(const char *text) {
    uiEditableComboboxSetText(w_, text);
    return *this;
  }

  EditableCombobox on_changed(void (*fn)(uiEditableCombobox *sender, void *data), void *data) {
    uiEditableComboboxOnChanged(w_, fn, data);
    return *this;
  }
};

struct RadioButtons : Widget<RadioButtons, uiRadioButtons> {
  static RadioButtons make() { return wrap(uiNewRadioButtons()); }

  RadioButtons append(const char *text) {
    uiRadioButtonsAppend(w_, text);
    return *this;
  }

  int selected() const { return uiRadioButtonsSelected(w_); }

  RadioButtons set_selected(int index) {
    uiRadioButtonsSetSelected(w_, index);
    return *this;
  }

  RadioButtons on_selected(void (*fn)(uiRadioButtons *sender, void *data), void *data) {
    uiRadioButtonsOnSelected(w_, fn, data);
    return *this;
  }
};

struct DateTimePicker : Widget<DateTimePicker, uiDateTimePicker> {
  static DateTimePicker make() { return wrap(uiNewDateTimePicker()); }

  static DateTimePicker make_date() { return wrap(uiNewDatePicker()); }

  static DateTimePicker make_time() { return wrap(uiNewTimePicker()); }

  void time(struct tm *out) const { uiDateTimePickerTime(w_, out); }

  DateTimePicker set_time(const struct tm *time) {
    uiDateTimePickerSetTime(w_, time);
    return *this;
  }

  DateTimePicker on_changed(void (*fn)(uiDateTimePicker *sender, void *data), void *data) {
    uiDateTimePickerOnChanged(w_, fn, data);
    return *this;
  }
};

struct MultilineEntry : Widget<MultilineEntry, uiMultilineEntry> {
  static MultilineEntry make() { return wrap(uiNewMultilineEntry()); }

  static MultilineEntry make_non_wrapping() { return wrap(uiNewNonWrappingMultilineEntry()); }

  Text text() const { return detail::adopt_text(uiMultilineEntryText(w_)); }

  MultilineEntry set_text(const char *text) {
    uiMultilineEntrySetText(w_, text);
    return *this;
  }

  MultilineEntry append(const char *text) {
    uiMultilineEntryAppend(w_, text);
    return *this;
  }

  bool readonly() const { return uiMultilineEntryReadOnly(w_) != 0; }

  MultilineEntry readonly(bool value) {
    uiMultilineEntrySetReadOnly(w_, value ? 1 : 0);
    return *this;
  }

  MultilineEntry on_changed(void (*fn)(uiMultilineEntry *sender, void *data), void *data) {
    uiMultilineEntryOnChanged(w_, fn, data);
    return *this;
  }
};

struct ColorButton : Widget<ColorButton, uiColorButton> {
  static ColorButton make() { return wrap(uiNewColorButton()); }

  void color(double *r, double *g, double *b, double *a) const {
    uiColorButtonColor(w_, r, g, b, a);
  }

  ColorButton set_color(double r, double g, double b, double a) {
    uiColorButtonSetColor(w_, r, g, b, a);
    return *this;
  }

  ColorButton on_changed(void (*fn)(uiColorButton *sender, void *data), void *data) {
    uiColorButtonOnChanged(w_, fn, data);
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
  static FontButton make() { return wrap(uiNewFontButton()); }

  FontDescriptor font() const {
    FontDescriptor result;
    uiFontButtonFont(w_, &result.desc);
    result.from_button_ = true;
    return result;
  }

  FontButton on_changed(void (*fn)(uiFontButton *sender, void *data), void *data) {
    uiFontButtonOnChanged(w_, fn, data);
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

class DrawBrush {
 public:
  DrawBrush() = default;

  static DrawBrush solid(double r, double g, double b, double a) {
    DrawBrush result;
    result.brush_.Type = uiDrawBrushTypeSolid;
    result.brush_.R = r;
    result.brush_.G = g;
    result.brush_.B = b;
    result.brush_.A = a;
    return result;
  }

  static DrawBrush linear_gradient(double x0, double y0, double x1, double y1,
                                   std::vector<uiDrawBrushGradientStop> stops) {
    DrawBrush result;
    result.brush_.Type = uiDrawBrushTypeLinearGradient;
    result.brush_.X0 = x0;
    result.brush_.Y0 = y0;
    result.brush_.X1 = x1;
    result.brush_.Y1 = y1;
    result.stops_ = std::move(stops);
    return result;
  }

  static DrawBrush linear_gradient(double x0, double y0, double x1, double y1,
                                   std::initializer_list<uiDrawBrushGradientStop> stops) {
    return linear_gradient(x0, y0, x1, y1, std::vector<uiDrawBrushGradientStop>(stops));
  }

  static DrawBrush radial_gradient(double x0, double y0, double x1, double y1, double outer_radius,
                                   std::vector<uiDrawBrushGradientStop> stops) {
    DrawBrush result;
    result.brush_.Type = uiDrawBrushTypeRadialGradient;
    result.brush_.X0 = x0;
    result.brush_.Y0 = y0;
    result.brush_.X1 = x1;
    result.brush_.Y1 = y1;
    result.brush_.OuterRadius = outer_radius;
    result.stops_ = std::move(stops);
    return result;
  }

  static DrawBrush radial_gradient(double x0, double y0, double x1, double y1, double outer_radius,
                                   std::initializer_list<uiDrawBrushGradientStop> stops) {
    return radial_gradient(x0, y0, x1, y1, outer_radius,
                           std::vector<uiDrawBrushGradientStop>(stops));
  }

  DrawBrush(const DrawBrush &) = default;
  DrawBrush &operator=(const DrawBrush &) = default;
  DrawBrush(DrawBrush &&) noexcept = default;
  DrawBrush &operator=(DrawBrush &&) noexcept = default;

  uiDrawBrush *raw() {
    sync_stops();
    return &brush_;
  }

  const uiDrawBrush *raw() const {
    sync_stops();
    return &brush_;
  }

 private:
  mutable uiDrawBrush brush_{};
  std::vector<uiDrawBrushGradientStop> stops_;

  void sync_stops() const {
    if (stops_.empty()) {
      brush_.Stops = nullptr;
      brush_.NumStops = 0;
    } else {
      brush_.Stops = const_cast<uiDrawBrushGradientStop *>(stops_.data());
      brush_.NumStops = stops_.size();
    }
  }
};

class DrawPath {
 public:
  DrawPath() = default;

  static DrawPath make(uiDrawFillMode fill_mode = uiDrawFillModeWinding) {
    DrawPath result;
    result.path_ = uiDrawNewPath(fill_mode);
    return result;
  }

  ~DrawPath() { reset(); }

  DrawPath(const DrawPath &) = delete;
  DrawPath &operator=(const DrawPath &) = delete;

  DrawPath(DrawPath &&other) noexcept : path_(other.path_) { other.path_ = nullptr; }

  DrawPath &operator=(DrawPath &&other) noexcept {
    if (this != &other) {
      reset();
      path_ = other.path_;
      other.path_ = nullptr;
    }
    return *this;
  }

  DrawPath &new_figure(double x, double y) {
    uiDrawPathNewFigure(path_, x, y);
    return *this;
  }

  DrawPath &new_figure_with_arc(double x_center, double y_center, double radius, double start_angle,
                                double sweep, bool negative = false) {
    uiDrawPathNewFigureWithArc(path_, x_center, y_center, radius, start_angle, sweep,
                               negative ? 1 : 0);
    return *this;
  }

  DrawPath &line_to(double x, double y) {
    uiDrawPathLineTo(path_, x, y);
    return *this;
  }

  DrawPath &arc_to(double x_center, double y_center, double radius, double start_angle,
                   double sweep, bool negative = false) {
    uiDrawPathArcTo(path_, x_center, y_center, radius, start_angle, sweep, negative ? 1 : 0);
    return *this;
  }

  DrawPath &bezier_to(double c1x, double c1y, double c2x, double c2y, double end_x, double end_y) {
    uiDrawPathBezierTo(path_, c1x, c1y, c2x, c2y, end_x, end_y);
    return *this;
  }

  DrawPath &close_figure() {
    uiDrawPathCloseFigure(path_);
    return *this;
  }

  DrawPath &add_rectangle(double x, double y, double width, double height) {
    uiDrawPathAddRectangle(path_, x, y, width, height);
    return *this;
  }

  bool ended() const { return uiDrawPathEnded(path_) != 0; }

  DrawPath &end() {
    uiDrawPathEnd(path_);
    return *this;
  }

  uiDrawPath *raw() const { return path_; }

 private:
  uiDrawPath *path_ = nullptr;

  void reset() {
    if (path_ != nullptr) {
      uiDrawFreePath(path_);
      path_ = nullptr;
    }
  }
};

class DrawContext {
 public:
  DrawContext() = default;

  static DrawContext wrap(uiDrawContext *ctx) {
    DrawContext result;
    result.ctx_ = ctx;
    return result;
  }

  DrawContext &fill(const DrawPath &path, const DrawBrush &brush) {
    uiDrawFill(ctx_, path.raw(), const_cast<uiDrawBrush *>(brush.raw()));
    return *this;
  }

  DrawContext &stroke(const DrawPath &path, const DrawBrush &brush,
                      const uiDrawStrokeParams &params) {
    uiDrawStroke(ctx_, path.raw(), const_cast<uiDrawBrush *>(brush.raw()),
                 const_cast<uiDrawStrokeParams *>(&params));
    return *this;
  }

  DrawContext &clip(const DrawPath &path) {
    uiDrawClip(ctx_, path.raw());
    return *this;
  }

  DrawContext &transform(const uiDrawMatrix &matrix) {
    uiDrawTransform(ctx_, const_cast<uiDrawMatrix *>(&matrix));
    return *this;
  }

  DrawContext &save() {
    uiDrawSave(ctx_);
    return *this;
  }

  DrawContext &restore() {
    uiDrawRestore(ctx_);
    return *this;
  }

  uiDrawContext *raw() const { return ctx_; }

 private:
  uiDrawContext *ctx_ = nullptr;
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

  static TableValue wrap(const uiTableValue *value) {
    TableValue result;
    result.value_ = const_cast<uiTableValue *>(value);
    result.owns_ = false;
    return result;
  }

 private:
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

struct TableModelHandler : uiTableModelHandler {
  uiTableModelHandler *raw() { return this; }

  const uiTableModelHandler *raw() const { return this; }
};

class TableModel {
 public:
  TableModel() = default;

  static TableModel make(TableModelHandler &handler) {
    return wrap(uiNewTableModel(handler.raw()));
  }

  static TableModel wrap(uiTableModel *raw) {
    TableModel result;
    result.model_ = raw;
    return result;
  }

  uiTableModel *raw() const { return model_; }

  void free() {
    if (model_ != nullptr) {
      uiFreeTableModel(model_);
      model_ = nullptr;
    }
  }

  void row_inserted(int index) { uiTableModelRowInserted(model_, index); }

  void row_changed(int index) { uiTableModelRowChanged(model_, index); }

  void row_deleted(int index) { uiTableModelRowDeleted(model_, index); }

 private:
  uiTableModel *model_ = nullptr;
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
    return wrap(uiNewTable(&p));
  }

  Table append_text_column(const char *name, int text_model_column, int text_editable_model_column,
                           const TableTextColumnParams *text_params = nullptr) {
    uiTableTextColumnOptionalParams tp{};
    uiTableTextColumnOptionalParams *tp_ptr = nullptr;
    if (text_params != nullptr) {
      tp.ColorModelColumn = text_params->color_model_column;
      tp_ptr = &tp;
    }
    uiTableAppendTextColumn(w_, name, text_model_column, text_editable_model_column, tp_ptr);
    return *this;
  }

  Table append_image_column(const char *name, int image_model_column) {
    uiTableAppendImageColumn(w_, name, image_model_column);
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
    uiTableAppendImageTextColumn(w_, name, image_model_column, text_model_column,
                                 text_editable_model_column, tp_ptr);
    return *this;
  }

  Table append_checkbox_column(const char *name, int checkbox_model_column,
                               int checkbox_editable_model_column) {
    uiTableAppendCheckboxColumn(w_, name, checkbox_model_column, checkbox_editable_model_column);
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
    uiTableAppendCheckboxTextColumn(w_, name, checkbox_model_column, checkbox_editable_model_column,
                                    text_model_column, text_editable_model_column, tp_ptr);
    return *this;
  }

  Table append_progress_bar_column(const char *name, int progress_model_column) {
    uiTableAppendProgressBarColumn(w_, name, progress_model_column);
    return *this;
  }

  Table append_button_column(const char *name, int button_model_column,
                             int button_clickable_model_column) {
    uiTableAppendButtonColumn(w_, name, button_model_column, button_clickable_model_column);
    return *this;
  }

  bool header_visible() const { return uiTableHeaderVisible(w_) != 0; }

  Table header_visible(bool value) {
    uiTableHeaderSetVisible(w_, value ? 1 : 0);
    return *this;
  }

  Table header_sort_indicator(int column, uiSortIndicator indicator) {
    uiTableHeaderSetSortIndicator(w_, column, indicator);
    return *this;
  }

  uiSortIndicator header_sort_indicator(int column) const {
    return uiTableHeaderSortIndicator(w_, column);
  }

  int column_width(int column) const { return uiTableColumnWidth(w_, column); }

  Table set_column_width(int column, int width) {
    uiTableColumnSetWidth(w_, column, width);
    return *this;
  }

  uiTableSelectionMode selection_mode() const { return uiTableGetSelectionMode(w_); }

  Table set_selection_mode(uiTableSelectionMode mode) {
    uiTableSetSelectionMode(w_, mode);
    return *this;
  }

  TableSelection selection() const { return TableSelection(uiTableGetSelection(w_)); }

  Table set_selection(const std::vector<int> &rows) {
    uiTableSelection sel{};
    sel.NumRows = static_cast<int>(rows.size());
    sel.Rows = const_cast<int *>(rows.data());
    uiTableSetSelection(w_, &sel);
    return *this;
  }

  Table on_row_clicked(void (*fn)(uiTable *sender, int row, void *data), void *data) {
    uiTableOnRowClicked(w_, fn, data);
    return *this;
  }

  Table on_row_double_clicked(void (*fn)(uiTable *sender, int row, void *data), void *data) {
    uiTableOnRowDoubleClicked(w_, fn, data);
    return *this;
  }

  Table on_selection_changed(void (*fn)(uiTable *sender, void *data), void *data) {
    uiTableOnSelectionChanged(w_, fn, data);
    return *this;
  }

  Table header_on_clicked(void (*fn)(uiTable *sender, int column, void *data), void *data) {
    uiTableHeaderOnClicked(w_, fn, data);
    return *this;
  }
};

struct Separator : Widget<Separator, uiSeparator> {
  static Separator make_horizontal() { return wrap(uiNewHorizontalSeparator()); }

  static Separator make_vertical() { return wrap(uiNewVerticalSeparator()); }
};

struct Form : Widget<Form, uiForm> {
  static Form make() { return wrap(uiNewForm()); }

  bool padded() const { return uiFormPadded(w_) != 0; }

  Form padded(bool value) {
    uiFormSetPadded(w_, value ? 1 : 0);
    return *this;
  }

  int num_children() const { return uiFormNumChildren(w_); }

  Form delete_at(int index) {
    uiFormDelete(w_, index);
    return *this;
  }

  template <typename W>
  Form append(const char *label, W &child, bool stretchy = false) {
    uiFormAppend(w_, label, child.ctrl(), stretchy ? 1 : 0);
    return *this;
  }

  template <typename W>
  Form append(const char *label, W &&child, bool stretchy = false) {
    uiFormAppend(w_, label, std::forward<W>(child).ctrl(), stretchy ? 1 : 0);
    return *this;
  }

  template <typename W>
  Form append_stretchy(const char *label, W &child) {
    uiFormAppend(w_, label, child.ctrl(), 1);
    return *this;
  }

  template <typename W>
  Form append_stretchy(const char *label, W &&child) {
    uiFormAppend(w_, label, std::forward<W>(child).ctrl(), 1);
    return *this;
  }
};

struct Grid : Widget<Grid, uiGrid> {
  static Grid make() { return wrap(uiNewGrid()); }

  bool padded() const { return uiGridPadded(w_) != 0; }

  Grid padded(bool value) {
    uiGridSetPadded(w_, value ? 1 : 0);
    return *this;
  }

  template <typename W>
  Grid append(W &child, int left, int top, int xspan, int yspan, bool hexpand, uiAlign halign,
              bool vexpand, uiAlign valign) {
    uiGridAppend(w_, child.ctrl(), left, top, xspan, yspan, hexpand ? 1 : 0, halign,
                 vexpand ? 1 : 0, valign);
    return *this;
  }

  template <typename W>
  Grid append(W &&child, int left, int top, int xspan, int yspan, bool hexpand, uiAlign halign,
              bool vexpand, uiAlign valign) {
    uiGridAppend(w_, std::forward<W>(child).ctrl(), left, top, xspan, yspan, hexpand ? 1 : 0,
                 halign, vexpand ? 1 : 0, valign);
    return *this;
  }

  template <typename W>
  Grid insert_at(W &child, uiControl *existing, uiAt at, int xspan, int yspan, bool hexpand,
                 uiAlign halign, bool vexpand, uiAlign valign) {
    uiGridInsertAt(w_, child.ctrl(), existing, at, xspan, yspan, hexpand ? 1 : 0, halign,
                   vexpand ? 1 : 0, valign);
    return *this;
  }

  template <typename W>
  Grid insert_at(W &&child, uiControl *existing, uiAt at, int xspan, int yspan, bool hexpand,
                 uiAlign halign, bool vexpand, uiAlign valign) {
    uiGridInsertAt(w_, std::forward<W>(child).ctrl(), existing, at, xspan, yspan, hexpand ? 1 : 0,
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

struct AreaHandler : uiAreaHandler {
  uiAreaHandler *raw() { return this; }

  const uiAreaHandler *raw() const { return this; }
};

struct Area : Widget<Area, uiArea> {
  static Area make(AreaHandler &handler) { return wrap(uiNewArea(handler.raw())); }

  static Area make_scrolling(AreaHandler &handler, int width, int height) {
    return wrap(uiNewScrollingArea(handler.raw(), width, height));
  }

  Area copy() const {
    Area result;
    result.w_ = w_;
    return result;
  }

  Area set_size(int width, int height) {
    uiAreaSetSize(w_, width, height);
    return copy();
  }

  Area queue_redraw_all() {
    uiAreaQueueRedrawAll(w_);
    return copy();
  }

  Area scroll_to(double x, double y, double width, double height) {
    uiAreaScrollTo(w_, x, y, width, height);
    return copy();
  }

  void begin_user_window_move() { uiAreaBeginUserWindowMove(w_); }

  void begin_user_window_resize(uiWindowResizeEdge edge) { uiAreaBeginUserWindowResize(w_, edge); }
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
