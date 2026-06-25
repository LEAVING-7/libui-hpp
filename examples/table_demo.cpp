#include <cstdio>
#include <cstring>
#include <vector>

#include <ui/ui.hpp>

namespace {

ui::Image make_solid_image(int size, uint8_t r, uint8_t g, uint8_t b) {
  std::vector<uint8_t> pixels(static_cast<size_t>(size * size * 4));
  for (int i = 0; i < size * size; ++i) {
    pixels[static_cast<size_t>(i * 4) + 0] = r;
    pixels[static_cast<size_t>(i * 4) + 1] = g;
    pixels[static_cast<size_t>(i * 4) + 2] = b;
    pixels[static_cast<size_t>(i * 4) + 3] = 255;
  }
  ui::Image img = ui::Image::make(size, size);
  img.append(pixels.data(), size, size, size * 4);
  return img;
}

struct Page16Handler : ui::TableModelHandler {
  Page16Handler() {
    img_[0] = make_solid_image(16, 255, 0, 0);
    img_[1] = make_solid_image(16, 0, 128, 255);
    std::strcpy(row9text_, "Part");
    std::memset(check_states_, 0, sizeof(check_states_));

    NumColumns = [](uiTableModelHandler *, uiTableModel *) { return 9; };

    ColumnType = [](uiTableModelHandler *, uiTableModel *, int column) -> uiTableValueType {
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
    };

    NumRows = [](uiTableModelHandler *, uiTableModel *) { return 15; };

    CellValue = [](uiTableModelHandler *mh, uiTableModel *, int row, int column) -> uiTableValue * {
      auto *self = static_cast<Page16Handler *>(mh);
      char buf[256];

      if (column == 3) {
        if (row == self->yellow_row_) {
          return ui::TableValue::color(1, 1, 0, 1).release();
        }
        if (row == 3) {
          return ui::TableValue::color(1, 0, 0, 1).release();
        }
        if (row == 11) {
          return ui::TableValue::color(0, 0.5, 1, 0.5).release();
        }
        return nullptr;
      }
      if (column == 4) {
        if ((row % 2) == 1) {
          return ui::TableValue::color(0.5, 0, 0.75, 1).release();
        }
        return nullptr;
      }
      if (column == 5) {
        return ui::TableValue::image(row < 8 ? self->img_[0] : self->img_[1]).release();
      }
      if (column == 7) {
        return ui::TableValue::integer(self->check_states_[row]).release();
      }
      if (column == 8) {
        if (row == 0) {
          return ui::TableValue::integer(0).release();
        }
        if (row == 13) {
          return ui::TableValue::integer(100).release();
        }
        if (row == 14) {
          return ui::TableValue::integer(-1).release();
        }
        return ui::TableValue::integer(50).release();
      }

      switch (column) {
        case 0:
          std::snprintf(buf, sizeof(buf), "Row %d", row);
          break;
        case 2:
          if (row == 9) {
            return ui::TableValue::string(self->row9text_).release();
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
      return ui::TableValue::string(buf).release();
    };

    SetCellValue = [](uiTableModelHandler *mh, uiTableModel *m, int row, int column,
                      const uiTableValue *value) {
      auto *self = static_cast<Page16Handler *>(mh);
      ui::TableValue cell = ui::TableValue::wrap(value);

      if (row == 9 && column == 2 && !cell.empty()) {
        std::strncpy(self->row9text_, cell.as_string(), sizeof(self->row9text_) - 1);
        self->row9text_[sizeof(self->row9text_) - 1] = '\0';
      }
      if (column == 6) {
        int prev = self->yellow_row_;
        self->yellow_row_ = row;
        if (prev != -1) {
          ui::TableModel::wrap(m).row_changed(prev);
        }
        ui::TableModel::wrap(m).row_changed(self->yellow_row_);
      }
      if (column == 7 && !cell.empty()) {
        self->check_states_[row] = cell.as_int();
        std::printf("Checkbox[%d] = %d\n", row, self->check_states_[row]);
      }
    };
  }

  const int *check_states() const { return check_states_; }

 private:
  ui::Image img_[2];
  char row9text_[1024];
  int yellow_row_ = -1;
  int check_states_[15];
};

struct Context {
  Page16Handler handler;
  ui::TableModel table_model;
  ui::Table table_;
  ui::Label lbl_row_clicked;
  ui::Label lbl_row_double_clicked;
  ui::Label lbl_num_selected_rows;
  ui::Label lbl_sum_selected_rows;
  ui::Label lbl_count_selection_changed;
  ui::Spinbox column_id;
  ui::Spinbox column_width;
  int count_selection_changed = 0;
  int header_sort_prev_ = 0;

  Context() : table_model(ui::TableModel::make(handler)) {}

  ~Context() { table_model.free(); }

  ui::VerticalBox make_page();

  int on_window_closing(ui::Window window) {
    (void)window;
    ui::Application::quit();
    return 1;
  }

 private:
  void on_header_visible_toggled(ui::Checkbox checkbox) {
    table_.header_visible(checkbox.checked());
    checkbox.set_checked(table_.header_visible());
  }

  void on_selection_mode_selected(ui::Combobox combobox) {
    int index = combobox.selected();
    if (index < 0) {
      return;
    }
    table_.set_selection_mode(static_cast<uiTableSelectionMode>(index));
    combobox.set_selected(static_cast<int>(table_.selection_mode()));
  }

  void on_column_id_changed(ui::Spinbox spinbox) {
    column_width.set_value(table_.column_width(spinbox.value()));
  }

  void on_column_width_changed(ui::Spinbox spinbox) {
    table_.set_column_width(column_id.value(), spinbox.value());
  }

  void on_row_clicked(ui::Table table, int row) {
    (void)table;
    char str[128];
    std::printf("Clicked row %d\n", row);
    std::snprintf(str, sizeof(str), "Clicked row %d", row);
    lbl_row_clicked.set_text(str);
  }

  void on_row_double_clicked(ui::Table table, int row) {
    (void)table;
    char str[128];
    std::printf("Double clicked row %d\n", row);
    std::snprintf(str, sizeof(str), "Double clicked row %d", row);
    lbl_row_double_clicked.set_text(str);
  }

  void header_on_clicked(ui::Table table, int col) {
    if (header_sort_prev_ != col) {
      table.header_sort_indicator(header_sort_prev_, uiSortIndicatorNone);
    }

    if (table.header_sort_indicator(col) == uiSortIndicatorAscending) {
      table.header_sort_indicator(col, uiSortIndicatorDescending);
    } else {
      table.header_sort_indicator(col, uiSortIndicatorAscending);
    }
    header_sort_prev_ = col;
  }

  void on_selection_changed(ui::Table table) {
    ui::TableSelection selection = table.selection();

    char str[128];
    std::snprintf(str, sizeof(str), "# Selection Changed Count: %d", ++count_selection_changed);
    lbl_count_selection_changed.set_text(str);

    std::snprintf(str, sizeof(str), "# Selected Rows: %d", selection.num_rows());
    lbl_num_selected_rows.set_text(str);

    int sum = 0;
    for (int row : selection.rows()) {
      sum += row;
    }
    std::snprintf(str, sizeof(str), "Sum Selected Rows: %d", sum);
    lbl_sum_selected_rows.set_text(str);
  }

  void on_select_checked(ui::Button sender) {
    (void)sender;
    std::vector<int> rows;
    for (int i = 0; i < 15; ++i) {
      if (handler.check_states()[i]) {
        rows.push_back(i);
      }
    }
    table_.set_selection(rows);
  }
};

ui::VerticalBox Context::make_page() {
  ui::Table::Params params;
  params.model = &table_model;
  params.row_background_color_model_column = 3;
  table_ = ui::Table::make(params);

  uiTableTextColumnOptionalParams text_params{4};

  table_.append_text_column("Column 1", 0, ui::kTableModelColumnNeverEditable)
      .append_image_text_column("Column 2", 5, 1, ui::kTableModelColumnNeverEditable, text_params)
      .append_text_column("Editable", 2, ui::kTableModelColumnAlwaysEditable)
      .append_checkbox_column("Checkboxes", 7, ui::kTableModelColumnAlwaysEditable)
      .append_button_column("Buttons", 6, ui::kTableModelColumnAlwaysEditable)
      .append_progress_bar_column("Progress Bar", 8)
      .header_on_clicked<Context, &Context::header_on_clicked>(this)
      .on_selection_changed<Context, &Context::on_selection_changed>(this)
      .on_row_clicked<Context, &Context::on_row_clicked>(this)
      .on_row_double_clicked<Context, &Context::on_row_double_clicked>(this);

  lbl_row_clicked = ui::Label::make("Clicked row -");
  lbl_row_double_clicked = ui::Label::make("Double clicked row -");
  lbl_num_selected_rows = ui::Label::make("");
  lbl_sum_selected_rows = ui::Label::make("");
  lbl_count_selection_changed = ui::Label::make("");

  column_id = ui::Spinbox::make(0, 5);
  column_width = ui::Spinbox::make(-1, 1000000);

  return ui::VerticalBox::make()
      .append(ui::HorizontalBox::make()
                  .padded(true)
                  .append(ui::Checkbox::make("Header Visible")
                              .set_checked(table_.header_visible())
                              .on_toggled<Context, &Context::on_header_visible_toggled>(this))
                  .append(ui::Separator::make_vertical())
                  .append(ui::Combobox::make()
                              .append("None")
                              .append("ZeroOrOne")
                              .append("One")
                              .append("ZeroOrMany")
                              .set_selected(static_cast<int>(table_.selection_mode()))
                              .on_selected<Context, &Context::on_selection_mode_selected>(this))
                  .append(ui::Separator::make_vertical())
                  .append(ui::Label::make("Column"))
                  .append(column_id.on_changed<Context, &Context::on_column_id_changed>(this))
                  .append(ui::Label::make("Width"))
                  .append(column_width.on_changed<Context, &Context::on_column_width_changed>(this))
                  .append(ui::Separator::make_vertical())
                  .append(ui::Button::make("Select Checked")
                              .on_clicked<Context, &Context::on_select_checked>(this)))
      .append(table_, true)
      .append(ui::HorizontalBox::make()
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
  if (!ui::Application::init(&err)) {
    std::fprintf(stderr, "error initializing libui: %s\n", err.c_str());
    return 1;
  }

  {
    Context ctx;
    ui::Window window = ui::Window::make("libui Table Demo", 800, 600, false);
    window.on_closing<Context, &Context::on_window_closing>(&ctx);
    window.set_child(ctx.make_page()).margined(true).show();
    ui::Application::run();
  }
  ui::Application::uninit();
  return 0;
}
