#include <cstdio>
#include <cstring>
#include <vector>

#include <ui.hpp>

namespace {

ui::Image make_solid_image(int size, uint8_t r, uint8_t g, uint8_t b) {
  std::vector<uint8_t> pixels(static_cast<size_t>(size * size * 4));
  for (int i = 0; i < size * size; ++i) {
    pixels[static_cast<size_t>(i * 4) + 0] = r;
    pixels[static_cast<size_t>(i * 4) + 1] = g;
    pixels[static_cast<size_t>(i * 4) + 2] = b;
    pixels[static_cast<size_t>(i * 4) + 3] = 255;
  }
  ui::Image img = ui::Image::New(size, size);
  img.append(pixels.data(), size, size, size * 4);
  return img;
}

class Page16Model : public ui::TableModelHandler {
 public:
  Page16Model() {
    img_[0] = make_solid_image(16, 255, 0, 0);
    img_[1] = make_solid_image(16, 0, 128, 255);
    std::strcpy(row9text_, "Part");
    std::memset(check_states_, 0, sizeof(check_states_));
  }

  int num_columns() override { return 9; }

  uiTableValueType column_type(int column) override {
    if (column == 3 || column == 4) {
      return uiTableValueTypeColor;
    }
    if (column == 5) {
      return uiTableValueTypeImage;
    }
    if (column == 7 || column == 8) {
      return uiTableValueTypeInt;
    }
    return uiTableValueTypeString;
  }

  int num_rows() override { return 15; }

  ui::TableValue cell_value(int row, int column) override {
    char buf[256];

    if (column == 3) {
      if (row == yellow_row_) {
        return ui::TableValue::color(1, 1, 0, 1);
      }
      if (row == 3) {
        return ui::TableValue::color(1, 0, 0, 1);
      }
      if (row == 11) {
        return ui::TableValue::color(0, 0.5, 1, 0.5);
      }
      return {};
    }
    if (column == 4) {
      if ((row % 2) == 1) {
        return ui::TableValue::color(0.5, 0, 0.75, 1);
      }
      return {};
    }
    if (column == 5) {
      return ui::TableValue::image(row < 8 ? img_[0] : img_[1]);
    }
    if (column == 7) {
      return ui::TableValue::integer(check_states_[row]);
    }
    if (column == 8) {
      if (row == 0) {
        return ui::TableValue::integer(0);
      }
      if (row == 13) {
        return ui::TableValue::integer(100);
      }
      if (row == 14) {
        return ui::TableValue::integer(-1);
      }
      return ui::TableValue::integer(50);
    }

    switch (column) {
      case 0:
        std::snprintf(buf, sizeof(buf), "Row %d", row);
        break;
      case 2:
        if (row == 9) {
          return ui::TableValue::string(row9text_);
        }
        // fall through
      case 1:
        std::strcpy(buf, "Part");
        break;
      case 6:
        std::strcpy(buf, "Make Yellow");
        break;
      default:
        buf[0] = '\0';
        break;
    }
    return ui::TableValue::string(buf);
  }

  void set_cell_value(int row, int column, const ui::TableValue &value) override {
    if (row == 9 && column == 2 && !value.empty()) {
      std::strncpy(row9text_, value.as_string(), sizeof(row9text_) - 1);
      row9text_[sizeof(row9text_) - 1] = '\0';
    }
    if (column == 6) {
      int prev = yellow_row_;
      yellow_row_ = row;
      if (model_ != nullptr) {
        if (prev != -1) {
          model_->row_changed(prev);
        }
        model_->row_changed(yellow_row_);
      }
    }
    if (column == 7 && !value.empty()) {
      check_states_[row] = value.as_int();
      std::printf("Checkbox[%d] = %d\n", row, check_states_[row]);
    }
  }

  void bind(ui::TableModel &model) { model_ = &model; }

  const int *check_states() const { return check_states_; }

 private:
  ui::Image img_[2];
  char row9text_[1024];
  int yellow_row_ = -1;
  int check_states_[15];
  ui::TableModel *model_ = nullptr;
};

struct Context {
  Page16Model page_model;
  ui::TableModel table_model;
  uiTable *table = nullptr;
  ui::Label lbl_row_clicked;
  ui::Label lbl_row_double_clicked;
  ui::Label lbl_num_selected_rows;
  ui::Label lbl_sum_selected_rows;
  ui::Label lbl_count_selection_changed;
  ui::Spinbox column_id;
  ui::Spinbox column_width;
  int count_selection_changed = 0;

  Context() : table_model(ui::TableModel::New(page_model)) { page_model.bind(table_model); }

  ui::VerticalBox make_page();
};

static int on_closing(uiWindow *, void *) {
  uiQuit();
  return 1;
}

static void header_visible_toggled(uiCheckbox *sender, void *data) {
  auto *ctx = static_cast<Context *>(data);
  ui::Checkbox checkbox;
  checkbox.w = sender;
  ui::Table table;
  table.w = ctx->table;
  table.header_visible(checkbox.checked());
  checkbox.set_checked(table.header_visible());
}

static void selection_mode_on_selected(uiCombobox *sender, void *data) {
  auto *ctx = static_cast<Context *>(data);
  ui::Combobox combobox;
  combobox.w = sender;
  ui::Table table;
  table.w = ctx->table;
  int index = combobox.selected();
  if (index < 0) {
    return;
  }
  table.set_selection_mode(static_cast<uiTableSelectionMode>(index));
  combobox.set_selected(static_cast<int>(table.selection_mode()));
}

static void changed_column_id(uiSpinbox *sender, void *data) {
  auto *ctx = static_cast<Context *>(data);
  ui::Spinbox spinbox;
  spinbox.w = sender;
  ui::Table table;
  table.w = ctx->table;
  ctx->column_width.set_value(table.column_width(spinbox.value()));
}

static void changed_column_width(uiSpinbox *sender, void *data) {
  auto *ctx = static_cast<Context *>(data);
  ui::Spinbox spinbox;
  spinbox.w = sender;
  ui::Table table;
  table.w = ctx->table;
  table.set_column_width(ctx->column_id.value(), spinbox.value());
}

static void on_row_clicked(uiTable *table, int row, void *data) {
  auto *ctx = static_cast<Context *>(data);
  char str[128];
  std::printf("Clicked row %d\n", row);
  std::snprintf(str, sizeof(str), "Clicked row %d", row);
  ctx->lbl_row_clicked.set_text(str);
  (void)table;
}

static void on_row_double_clicked(uiTable *table, int row, void *data) {
  auto *ctx = static_cast<Context *>(data);
  char str[128];
  std::printf("Double clicked row %d\n", row);
  std::snprintf(str, sizeof(str), "Double clicked row %d", row);
  ctx->lbl_row_double_clicked.set_text(str);
  (void)table;
}

static void header_on_clicked(uiTable *sender, int col, void *) {
  static int prev = 0;
  ui::Table table;
  table.w = sender;

  if (prev != col) {
    table.header_sort_indicator(prev, uiSortIndicatorNone);
  }

  if (table.header_sort_indicator(col) == uiSortIndicatorAscending) {
    table.header_sort_indicator(col, uiSortIndicatorDescending);
  } else {
    table.header_sort_indicator(col, uiSortIndicatorAscending);
  }
  prev = col;
}

static void on_selection_changed(uiTable *sender, void *data) {
  auto *ctx = static_cast<Context *>(data);
  ui::Table table;
  table.w = sender;
  ui::TableSelection selection = table.selection();

  char str[128];
  std::snprintf(str, sizeof(str), "# Selection Changed Count: %d", ++ctx->count_selection_changed);
  ctx->lbl_count_selection_changed.set_text(str);

  std::snprintf(str, sizeof(str), "# Selected Rows: %d", selection.num_rows());
  ctx->lbl_num_selected_rows.set_text(str);

  int sum = 0;
  for (int row : selection.rows()) {
    sum += row;
  }
  std::snprintf(str, sizeof(str), "Sum Selected Rows: %d", sum);
  ctx->lbl_sum_selected_rows.set_text(str);
}

static void select_checked(uiButton *, void *data) {
  auto *ctx = static_cast<Context *>(data);
  ui::Table table;
  table.w = ctx->table;
  std::vector<int> rows;
  for (int i = 0; i < 15; ++i) {
    if (ctx->page_model.check_states()[i]) {
      rows.push_back(i);
    }
  }
  table.set_selection(rows);
}

ui::VerticalBox Context::make_page() {
  ui::Table::Params params;
  params.model = &table_model;
  params.row_background_color_model_column = 3;
  ui::Table table = ui::Table::New(params);
  this->table = table.raw();

  ui::TableTextColumnParams text_params;
  text_params.color_model_column = 4;

  table.append_text_column("Column 1", 0, ui::kTableModelColumnNeverEditable)
      .append_image_text_column("Column 2", 5, 1, ui::kTableModelColumnNeverEditable, &text_params)
      .append_text_column("Editable", 2, ui::kTableModelColumnAlwaysEditable)
      .append_checkbox_column("Checkboxes", 7, ui::kTableModelColumnAlwaysEditable)
      .append_button_column("Buttons", 6, ui::kTableModelColumnAlwaysEditable)
      .append_progress_bar_column("Progress Bar", 8)
      .header_on_clicked(header_on_clicked, nullptr)
      .on_selection_changed(on_selection_changed, this)
      .on_row_clicked(on_row_clicked, this)
      .on_row_double_clicked(on_row_double_clicked, this);

  lbl_row_clicked = ui::Label::New("Clicked row -");
  lbl_row_double_clicked = ui::Label::New("Double clicked row -");
  lbl_num_selected_rows = ui::Label::New("");
  lbl_sum_selected_rows = ui::Label::New("");
  lbl_count_selection_changed = ui::Label::New("");

  column_id = ui::Spinbox::New(0, 5);
  column_width = ui::Spinbox::New(-1, 1000000);

  return ui::VerticalBox::New(
      ui::HorizontalBox::New()
          .padded(true)
          .append(ui::Checkbox::New("Header Visible")
                      .set_checked(table.header_visible())
                      .on_toggled(header_visible_toggled, this))
          .append(ui::Separator::NewVertical())
          .append(ui::Combobox::New()
                      .append("None")
                      .append("ZeroOrOne")
                      .append("One")
                      .append("ZeroOrMany")
                      .set_selected(static_cast<int>(table.selection_mode()))
                      .on_selected(selection_mode_on_selected, this))
          .append(ui::Separator::NewVertical())
          .append(ui::Label::New("Column"))
          .append(column_id.on_changed(changed_column_id, this))
          .append(ui::Label::New("Width"))
          .append(column_width.on_changed(changed_column_width, this))
          .append(ui::Separator::NewVertical())
          .append(ui::Button::New("Select Checked").on_clicked(select_checked, this)),
      table,
      ui::HorizontalBox::New()
          .padded(true)
          .append(lbl_row_clicked)
          .append(lbl_row_double_clicked)
          .append(lbl_num_selected_rows)
          .append(lbl_sum_selected_rows)
          .append(lbl_count_selection_changed));
}

}  // namespace

int main() {
  std::string err;
  if (!ui::Application::Init(&err)) {
    std::fprintf(stderr, "error initializing libui: %s\n", err.c_str());
    return 1;
  }

  {
    Context ctx;
    ui::Window window = ui::Window::New("libui Table Demo", 800, 600, false);
    window.on_closing(on_closing, nullptr);
    window.set_child(ctx.make_page()).margined(true).show();
    ui::Application::run();
  }
  ui::Application::Uninit();
  return 0;
}
