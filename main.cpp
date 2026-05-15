#include <FL/Fl.H>
#include <FL/Fl_Text_Display.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Tabs.H>
#include <FL/Fl_Group.H>
#include <FL/Enumerations.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Native_File_Chooser.H>
#include <FL/fl_ask.H>
#include <FL/Fl_Table_Row.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Float_Input.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Return_Button.H>

#include <vector>
#include <fstream>
#include <string>
#include <sstream>
#include <cmath>
#include <chrono>
#include <utility>
#include <algorithm>
#include <unordered_map>
#include <iostream>
#include <map>

struct RowData {
    std::string name;
    std::string group_name;

    std::string x, y;
    long double distance;
    std::string group_distance;

    std::string type;
    std::string group_type;

    std::string date;
    std::string group_date;
};

class InputDialog : public Fl_Window {
    Fl_Input* name_in;
    Fl_Input* x_in;
    Fl_Input* y_in;
    Fl_Input* type_ch;
    Fl_Input* date_in;

    bool _cancelled = true;

public:
    InputDialog() : Fl_Window(320, 260, "Ввод данных") {
        set_modal();
        
        int y = 20, h = 25, w = 180, x = 110;

        name_in = new Fl_Input(x, y, w, h, "Имя:");
        y += 35;
        
        x_in = new Fl_Input(x, y, w, h, "Коорд. X:");
        y += 35;
        
        y_in = new Fl_Input(x, y, w, h, "Коорд. Y:");
        y += 35;
        
        type_ch = new Fl_Input(x, y, w, h, "Тип:");
        y += 35;
        
        date_in = new Fl_Input(x, y, w, h, "Время:");
        y += 35;

        Fl_Return_Button* ok = new Fl_Return_Button(110, y, 90, 30, "OK");
        ok->callback(ok_cb, this);

        Fl_Button* cancel = new Fl_Button(210, y, 80, 30, "Отмена");
        cancel->callback(cancel_cb, this);

        end();
    }

    static void ok_cb(Fl_Widget*, void* v) {
        InputDialog* d = (InputDialog*)v;
        d->_cancelled = false;
        d->hide();
    }

    static void cancel_cb(Fl_Widget*, void* v) {
        ((InputDialog*)v)->hide();
    }

    bool is_cancelled() const { return _cancelled; }

    const char* getName()  { return name_in->value(); }
    const char* getX()     { return x_in->value(); }
    const char* getY()     { return y_in->value(); }
    const char* getType()  { return type_ch->value(); }
    const char* getDate()  { return date_in->value(); }
};

class Data_Table : public Fl_Table_Row {

    public:

    std::vector<RowData> rows_data;
    bool is_changed = false;
    bool is_new = false;
    std::string filaname;

    Data_Table(int x, int y, int w, int h, const char* l = 0) : Fl_Table_Row(x, y, w, h, l) {
        cols(headers.size());
        col_header(1);
        col_resize(1);
        end();
    }

    void add_data(const std::vector<RowData>& new_data) {
        rows_data = new_data;
        rows(new_data.size());
        redraw();
    }

    void auto_size_columns() {
        fl_font(FL_HELVETICA, FL_NORMAL_SIZE); 

        for (int c = 0; c < cols(); c++) {
            int x_len = 0; 
            int y_len = 0;

            std::string s;

            switch (c) {
                case 0:
                    s = rows_data[0].name; 
                    break;
                case 1: 
                    s = rows_data[0].x; 
                    break;
                case 2: 
                    s = rows_data[0].y; 
                    break;
                case 3: 
                    s = rows_data[0].type; 
                    break;
                case 4: 
                    s = rows_data[0].date; 
                    break;
            }

            fl_measure(s.c_str(), x_len, y_len);
        
            col_width(c, x_len + 20);
        }
    }

    protected:

    void draw_cell(TableContext context, int R = 0, int C = 0, int X = 0, int Y = 0, int W = 0, int H = 0) override {
        switch (context) {
            case CONTEXT_COL_HEADER: 
                fl_push_clip(X, Y, W, H);
                fl_draw_box(FL_THIN_UP_BOX, X, Y, W, H, col_header_color());
                fl_color(FL_BLACK);
                fl_draw(headers[C].c_str(), X, Y, W, H, FL_ALIGN_CENTER);
                fl_pop_clip();
                return;

            case CONTEXT_CELL: {    
                fl_push_clip(X, Y, W, H);
                fl_draw_box(FL_THIN_UP_BOX, X, Y, W, H, is_selected(R, C) ? FL_GREEN : FL_WHITE);
                fl_color(FL_BLACK);
                
                std::string s;
                if (R < (int)rows_data.size()) {
                    switch (C) {
                        case 0: 
                            s = rows_data[R].name; 
                            break;
                        case 1: 
                            s = rows_data[R].x; 
                            break;
                        case 2: 
                            s = rows_data[R].y; 
                            break;
                        case 3: 
                            s = rows_data[R].type; 
                            break;
                        case 4: 
                            s = rows_data[R].date; 
                            break;
                    }
                }
                fl_draw(s.c_str(), X, Y, W, H, FL_ALIGN_CENTER);
                fl_pop_clip();
                return;
            }
            default:
                return;
        }
    }

    private:

    const std::vector<std::string> headers = {"Имя", "Координата X", "Координата Y", "Тип", "Дата"};

};

class Data_Table_With_Distance : public Fl_Table_Row {

    public:

    std::vector<RowData> rows_data;
    std::string filaname;

    Data_Table_With_Distance(int x, int y, int w, int h, const char* l = 0) : Fl_Table_Row(x, y, w, h, l) {
        cols(headers.size());
        col_header(1);
        col_resize(1);
        end();
    }

    void add_data(const std::vector<RowData>& new_data) {
        rows_data = new_data;
        rows(new_data.size());
        redraw();
    }

    void auto_size_columns() {
        fl_font(FL_HELVETICA, FL_NORMAL_SIZE); 

        for (int c = 0; c < cols(); c++) {
            int x_len = 0; 
            int y_len = 0;

            std::string s;

            switch (c) {
                case 0: 
                    s = rows_data[0].name; 
                    break;
                case 1: 
                    s = rows_data[0].x; 
                    break;
                case 2: 
                    s = rows_data[0].y; 
                    break;
                case 3: {
                    std::stringstream sstream;
                    sstream.precision(2);
                    sstream << std::fixed << rows_data[0].distance;
                    s = sstream.str();
                    break;
                }
                case 4: 
                    s = rows_data[0].type; 
                    break;
                case 5:
                    s = rows_data[0].date;
                    break;
            }

            fl_measure(s.c_str(), x_len, y_len);
        
            col_width(c, x_len + 20);
        }
    }

    protected:

    void draw_cell(TableContext context, int R = 0, int C = 0, int X = 0, int Y = 0, int W = 0, int H = 0) override {
        switch (context) {
            case CONTEXT_COL_HEADER: 
                fl_push_clip(X, Y, W, H);
                fl_draw_box(FL_THIN_UP_BOX, X, Y, W, H, col_header_color());
                fl_color(FL_BLACK);
                fl_draw(headers[C].c_str(), X, Y, W, H, FL_ALIGN_CENTER);
                fl_pop_clip();
                return;

            case CONTEXT_CELL: {    
                fl_push_clip(X, Y, W, H);
                fl_draw_box(FL_THIN_UP_BOX, X, Y, W, H, is_selected(R, C) ? FL_GREEN : FL_WHITE);
                fl_color(FL_BLACK);
                
                std::string s;
                if (R < (int)rows_data.size()) {
                    switch (C) {
                        case 0: 
                            s = rows_data[R].name; 
                            break;
                        case 1: 
                            s = rows_data[R].x; 
                            break;
                        case 2: 
                            s = rows_data[R].y; 
                            break;
                        case 3: {
                            std::stringstream sstream;
                            sstream.precision(2);
                            sstream << std::fixed << rows_data[R].distance;
                            s = sstream.str();
                            break;
                        }
                        case 4: 
                            s = rows_data[R].type; 
                            break;
                        case 5:
                            s = rows_data[R].date;
                            break;
                    }
                }
                fl_draw(s.c_str(), X, Y, W, H, FL_ALIGN_CENTER);
                fl_pop_clip();
                return;
            }
            default:
                return;
        }
    }

    private:

    const std::vector<std::string> headers = {"Имя", "Координата X", "Координата Y", "Расстояние", "Тип", "Дата"};

};

class Main_Window : public Fl_Window {

    public:

    Main_Window() : Fl_Window {850, 480, "Тестовое задание"} {
      
        menu_bar.menu(items.data());
        menu_bar.parent(this);

        tabs_container = new Fl_Tabs(0, 30, 850, 450);
        tabs_container->end();

        resizable(tabs_container);
    }

    private:

    static void on_close_item_click(Fl_Widget *sender, void *data) {
        Main_Window* window = reinterpret_cast<Main_Window*>(data);
    
        Fl_Widget* current_tab = window->tabs_container->value();

        if (current_tab) {
            window->tabs_container->remove(current_tab);
        
            Fl::delete_widget(current_tab);
        
            window->tabs_container->redraw();
            window->redraw();
        } else {
            fl_alert("Нет открытых файлов!");
        }
    }

    static void on_menu_item_click(Fl_Widget* sender, void* data) {
        auto menu = reinterpret_cast<Fl_Menu_Bar*>(sender);
        Main_Window* window = reinterpret_cast<Main_Window*>(data);
        menu->value(0);
    }

    static void on_exit_item_click(Fl_Widget* sender, void* data) {
        Main_Window* window = reinterpret_cast<Main_Window*>(data);
        window->hide();
    }

    static void on_new_item_click(Fl_Widget* sender, void* data) {
        std::string filename = "Новый файл";
        Main_Window* window = reinterpret_cast<Main_Window*>(data);
        window->load_new_file(filename);
    }
    
    static void on_open_item_click(Fl_Widget* sender, void* data) {
        Fl_Native_File_Chooser file_chooser;

        file_chooser.title("Выберите файл");
        file_chooser.type(Fl_Native_File_Chooser::BROWSE_FILE);
        file_chooser.filter("All Files\t*");

        switch (file_chooser.show()) {
        case -1:
            fl_alert("Ошибка: %s", file_chooser.errmsg());
            break;
        case 1:  
            break;
        default: 
            Main_Window* window = reinterpret_cast<Main_Window*>(data);
            window->load_data(file_chooser.filename());
            break;
        }

    }

    static void on_save_item_click(Fl_Widget* sender, void* data) {
        Main_Window* window = reinterpret_cast<Main_Window*>(data);
        Fl_Widget* current_tab = window->tabs_container->value();

        if (!current_tab) {
            fl_alert("Нет открытых файлов!");
            return;
        }

        Fl_Group* tab_page = reinterpret_cast<Fl_Group*>(current_tab);
        if (tab_page->children() == 0) {
            fl_alert("Ошибка!");
            return;
        }

        if (window->is_grouped(data)) {
            
            Fl_Native_File_Chooser fnfc;
    
            fnfc.title("Сохранить файл как...");
            fnfc.type(Fl_Native_File_Chooser::BROWSE_SAVE_FILE);
            fnfc.filter("All Files\t*");
    
            fnfc.preset_file("file");

            Fl_Tabs* nested_tabs = nullptr;
            if (tab_page->children() > 0) {
                nested_tabs = reinterpret_cast<Fl_Tabs*>(tab_page->child(0));
            }

            switch (fnfc.show()) {
                case -1:
                    fl_alert("Ошибка: %s", fnfc.errmsg());
                    return;
                    break;
                case 1:
                    return;
                    break;
                default: {
                    window->save_grouped_file(fnfc.filename(), nested_tabs);
                    fl_alert("Сгруппированный файл сохранён!");
                    return;
                    break;
                }
            }
            
        }

        Data_Table *table = reinterpret_cast<Data_Table*>(tab_page->child(0));

        if (table->is_new) {

            Fl_Native_File_Chooser fnfc;
    
            fnfc.title("Сохранить файл как...");
            fnfc.type(Fl_Native_File_Chooser::BROWSE_SAVE_FILE);
            fnfc.filter("All Files\t*");
    
            fnfc.preset_file("file");

            switch (fnfc.show()) {
                case -1:
                    fl_alert("Ошибка: %s", fnfc.errmsg());
                    break;
                case 1:
                    break;
                default:
                    table->is_new = false;
                    window->save_file(fnfc.filename(), table->rows_data);
                    window->tabs_container->remove(current_tab);
        
                    Fl::delete_widget(current_tab);
        
                    window->tabs_container->redraw();
                    window->redraw();

                    window->load_data(fnfc.filename());
                    break;
            }

        } else if (table->is_changed) {

            table->is_changed = false;
            window->save_file(table->filaname, table->rows_data);

        } else {
            fl_alert("Нечего сохранять!");
        }
    }

    static void on_group_by_distance_item_click(Fl_Widget* sender, void* data) {
        Main_Window* window = reinterpret_cast<Main_Window*>(data);
        Fl_Widget* current_tab = window->tabs_container->value();

        if (window->is_grouped(data)) {
            fl_alert("Файл уже сгруппирован!");
            return;
        }

        if (!current_tab) {
            fl_alert("Нет открытых файлов для группировки!");
            return;
        }

        Fl_Group* file_tab_page = reinterpret_cast<Fl_Group*>(current_tab);
    
        Data_Table* old_table = reinterpret_cast<Data_Table*>(file_tab_page->child(0));

        std::vector<RowData> source_data = old_table->rows_data;
    
        std::map<std::string, std::vector<RowData>> groups;
        for (const auto& row : source_data) {
            groups[row.group_distance].push_back(row);
        }

        file_tab_page->begin();
    
        Fl_Tabs* nested_tabs = new Fl_Tabs(file_tab_page->x() + 5, file_tab_page->y() + 5, file_tab_page->w() - 10, file_tab_page->h() - 10);
    
        for (auto& pair : groups) {
            const std::string& group_label = pair.first;
            std::vector<RowData>& group_rows = pair.second;

            std::sort(group_rows.begin(), group_rows.end(), [](const RowData& a, const RowData& b) {
                return a.distance > b.distance;
            });

            Fl_Group* group_page = new Fl_Group(nested_tabs->x(), nested_tabs->y() + 25, nested_tabs->w(), nested_tabs->h() - 25, strdup(group_label.c_str()));
        
            Data_Table_With_Distance* dist_table = new Data_Table_With_Distance(group_page->x() + 5, group_page->y() + 5, group_page->w() - 10, group_page->h() - 10);
        
            dist_table->filaname = old_table->filaname;
            file_tab_page->remove(old_table);
            Fl::delete_widget(old_table);

            dist_table->add_data(group_rows);
            dist_table->auto_size_columns();
        
            group_page->end();
        }
    
        nested_tabs->end();
        file_tab_page->end();

        file_tab_page->redraw();
        window->redraw();
    }

    static void on_group_by_name_item_click(Fl_Widget* sender, void* data) {
        Main_Window* window = reinterpret_cast<Main_Window*>(data);
        Fl_Widget* current_tab = window->tabs_container->value();

        if (window->is_grouped(data)) {
            fl_alert("Файл уже сгруппирован!");
            return;
        }

        if (!current_tab) {
            fl_alert("Нет открытых файлов для группировки!");
            return;
        }

        Fl_Group* file_tab_page = reinterpret_cast<Fl_Group*>(current_tab);
    
        Data_Table* old_table = reinterpret_cast<Data_Table*>(file_tab_page->child(0));

        std::vector<RowData> source_data = old_table->rows_data;

        std::map<std::string, std::vector<RowData>> groups;
        for (const auto& row : source_data) {
            groups[row.group_name].push_back(row);
        }

        file_tab_page->begin();
    
        Fl_Tabs* nested_tabs = new Fl_Tabs(file_tab_page->x() + 5, file_tab_page->y() + 5, file_tab_page->w() - 10, file_tab_page->h() - 10);
    
        for (auto& pair : groups) {
            std::string latter = { pair.first, '\0' };
            std::vector<RowData>& group_rows = pair.second;

            std::sort(group_rows.begin(), group_rows.end(), [](const RowData& a, const RowData& b) {
                return a.name < b.name;
            });

            Fl_Group* group_page = new Fl_Group(nested_tabs->x(), nested_tabs->y() + 25, nested_tabs->w(), nested_tabs->h() - 25, strdup(latter.c_str()));
        
            Data_Table* name_table = new Data_Table(group_page->x() + 5, group_page->y() + 5, group_page->w() - 10, group_page->h() - 10);
        
            name_table->filaname = old_table->filaname;
            file_tab_page->remove(old_table);
            Fl::delete_widget(old_table);
            
            name_table->add_data(group_rows);
            name_table->auto_size_columns();
        
            group_page->end();
        }
    
        nested_tabs->end();
        file_tab_page->end();

        file_tab_page->redraw();
        window->redraw();
    }

    static void on_group_by_date_item_click(Fl_Widget* sender, void* data) {
        Main_Window* window = reinterpret_cast<Main_Window*>(data);
        Fl_Widget* current_tab = window->tabs_container->value();

        if (window->is_grouped(data)) {
            fl_alert("Файл уже сгруппирован!");
            return;
        }

        if (!current_tab) {
            fl_alert("Нет открытых файлов для группировки!");
            return;
        }

        Fl_Group* file_tab_page = reinterpret_cast<Fl_Group*>(current_tab);
    
        Data_Table* old_table = reinterpret_cast<Data_Table*>(file_tab_page->child(0));
        std::vector<RowData> source_data = old_table->rows_data;

        std::map<std::string, std::vector<RowData>> groups;
        for (const auto& row : source_data) {
            groups[row.group_date].push_back(row);
        }

        file_tab_page->begin();
    
        Fl_Tabs* nested_tabs = new Fl_Tabs(file_tab_page->x() + 5, file_tab_page->y() + 5, file_tab_page->w() - 10, file_tab_page->h() - 10);
    
        for (auto& pair : groups) {
            const std::string& group_label = pair.first;
            std::vector<RowData>& group_rows = pair.second;

            std::sort(group_rows.begin(), group_rows.end(), [](const RowData& a, const RowData& b) {
                try {
                    long double date_a = std::stold(a.date);
                    long double date_b = std::stold(b.date);
                    return date_a > date_b;
                } catch (const std::exception& e) {
                    return a.date > b.date;
                }
            });

            Fl_Group* group_page = new Fl_Group(nested_tabs->x(), nested_tabs->y() + 25, nested_tabs->w(), nested_tabs->h() - 25, strdup(group_label.c_str()));
        
            Data_Table* date_table = new Data_Table(group_page->x() + 5, group_page->y() + 5, group_page->w() - 10, group_page->h() - 10);
        
            date_table->filaname = old_table->filaname;
            file_tab_page->remove(old_table);
            Fl::delete_widget(old_table);

            date_table->add_data(group_rows);
            date_table->auto_size_columns();
        
            group_page->end();
        }
    
        nested_tabs->end();
        file_tab_page->end();

        file_tab_page->redraw();
        window->redraw();
    }

    static void on_group_by_type_item_click(Fl_Widget* sender, void* data) {
        Main_Window* window = reinterpret_cast<Main_Window*>(data);
        Fl_Widget* current_tab = window->tabs_container->value();

        if (window->is_grouped(data)) {
            fl_alert("Файл уже сгруппирован!");
            return;
        }

        if (!current_tab) {
            fl_alert("Нет открытых файлов для группировки!");
            return;
        }

        Fl_Group* file_tab_page = reinterpret_cast<Fl_Group*>(current_tab);
    
        Data_Table* old_table = reinterpret_cast<Data_Table*>(file_tab_page->child(0));

        std::vector<RowData> source_data = old_table->rows_data;


        std::map<std::string, std::vector<RowData>> groups;
        for (const auto& row : source_data) {
            groups[row.group_type].push_back(row);
        }

        file_tab_page->begin();
    
        Fl_Tabs* nested_tabs = new Fl_Tabs(file_tab_page->x() + 5, file_tab_page->y() + 5, file_tab_page->w() - 10, file_tab_page->h() - 10);
    
        for (auto& pair : groups) {
            const std::string& group_label = pair.first;
            std::vector<RowData>& group_rows = pair.second;

            std::sort(group_rows.begin(), group_rows.end(), [](const RowData& a, const RowData& b) {
                return a.name < b.name;
            });

            Fl_Group* group_page = new Fl_Group(nested_tabs->x(), nested_tabs->y() + 25, nested_tabs->w(), nested_tabs->h() - 25, strdup(group_label.c_str()));
        
            Data_Table* type_table = new Data_Table(group_page->x() + 5, group_page->y() + 5, group_page->w() - 10, group_page->h() - 10);
        
            type_table->filaname = old_table->filaname;
            type_table->add_data(group_rows);
            type_table->auto_size_columns();
        
            group_page->end();
        }

        file_tab_page->remove(old_table);
        Fl::delete_widget(old_table);
    
        nested_tabs->end();
        file_tab_page->end();

        file_tab_page->redraw();
        window->redraw();
    }

    static void on_ungroup_item_click(Fl_Widget* sender, void* data) {
        Main_Window* window = reinterpret_cast<Main_Window*>(data);
        Fl_Widget* current_tab = window->tabs_container->value();

        if (!window->is_grouped(data)) {
            fl_alert("Файл не сгруппирован!");
            return;
        }

        if (!current_tab) {
            fl_alert("Нет открытых вкладок!");
            return;
        }

        Fl_Group* file_tab_page = reinterpret_cast<Fl_Group*>(current_tab);

        Fl_Tabs* nested_tabs = nullptr;
        if (file_tab_page->children() > 0) {
            nested_tabs = reinterpret_cast<Fl_Tabs*>(file_tab_page->child(0));
        }

        std::vector<RowData> all_rows;
        std::string original_filename;

        Fl_Group* group_page = reinterpret_cast<Fl_Group*>(nested_tabs->child(0));

        auto* table1 = dynamic_cast<Data_Table*>(group_page->child(0));
        auto* table2 = dynamic_cast<Data_Table_With_Distance*>(group_page->child(0));

        if (table1) {
            original_filename = table1->filaname;
        } else if (table2) {
            original_filename = table2->filaname;
        }

        window->remove(file_tab_page);
        Fl::delete_widget(file_tab_page);
        
        window->load_data(original_filename);
    }

    static void on_add_item_click(Fl_Widget* sender, void* data) {
        Main_Window* window = reinterpret_cast<Main_Window*>(data);
        Fl_Widget* current_tab = window->tabs_container->value();

        if (window->is_grouped(data)) {
            fl_alert("Файл сгруппирован!");
            return;
        }

        if (!current_tab) {
            fl_alert("Нет открытых файлов!");
            return;
        }

        Fl_Group* tab_page = reinterpret_cast<Fl_Group*>(current_tab);
        if (tab_page->children() == 0) {
            fl_alert("Ошибка!");
            return;
        }

        Data_Table *table = reinterpret_cast<Data_Table*>(tab_page->child(0));
        std::vector<RowData> rows_data = table->rows_data;

        table->is_changed = true;

        InputDialog *dialog = new InputDialog();
        dialog->show();

        while (dialog->shown()) Fl::wait();

        if (!dialog->is_cancelled()) {
            RowData row;
            row.name = dialog->getName();
            row.type = dialog->getType();
            row.x = dialog->getX();
            row.y = dialog->getY();
            row.date = dialog->getDate();

            row = window->set_distance(row);
            row.group_name = window->set_group_name(row);
            row.group_date = window->set_group_date(row);

            rows_data.push_back(row);
        } 

        delete dialog;

        window->set_group_type(rows_data);
        table->add_data(rows_data);
    }

    void save_file(std::string filename, std::vector<RowData> rows_data) {

        std::ofstream file;
        file.open(filename);

        if(file.is_open()) {
            for (int i = 0; i < rows_data.size(); i++) {
                if (i != 0)
                    file << '\n';
                file << rows_data[i].name << ' ' << rows_data[i].x << ' ' << rows_data[i].y << ' ' << rows_data[i].type << ' ' << rows_data[i].date;
            }
        }

        file.close();

    }

    void save_grouped_file(std::string filename, Fl_Tabs *nested_tabs) {

        std::ofstream file;
        file.open(filename);

        if (file.is_open()) {
            for (int i = 0; i < nested_tabs->children(); i++) {
                Fl_Group *group_page = reinterpret_cast<Fl_Group*>(nested_tabs->child(i));
                if (!group_page) continue;

                file << "Группа " << (group_page->label() ? group_page->label() : "Без названия") << "\n";

                if (group_page->children() > 0) {

                    auto* table1 = dynamic_cast<Data_Table*>(group_page->child(0));
                    auto* table2 = dynamic_cast<Data_Table_With_Distance*>(group_page->child(0));

                    if (table1) {
                        std::vector<RowData> rows_data = table1->rows_data;
                        for (int j = 0; j < rows_data.size(); j++) {
                            file << rows_data[j].name << ' ' << rows_data[j].x << ' ' << rows_data[j].y << ' ' << rows_data[j].type << ' ' << rows_data[j].date << "\n";
                        }
                    } else if (table2) {
                        std::vector<RowData> rows_data = table2->rows_data;
                        for (int j = 0; j < rows_data.size(); j++) {
                            file << rows_data[j].name << ' ' << rows_data[j].x << ' ' << rows_data[j].y << ' ' << rows_data[j].distance <<  ' ' << rows_data[j].type << ' ' << rows_data[j].date << "\n";
                        }
                    }
                }
                file << "\n";
            }
        }

        file.close();
    }

    void load_new_file(std::string filename) {
        std::string label = filename;
        
        tabs_container->begin();
        Fl_Group* tab_page = new Fl_Group(tabs_container->x(), tabs_container->y() + 25, tabs_container->w(), tabs_container->h() - 25, strdup(label.c_str()));
        Data_Table* table = new Data_Table(tab_page->x() + 5, tab_page->y() + 5, tab_page->w() - 10, tab_page->h() - 10);
        table->is_new = true;
        tab_page->end();
        tabs_container->end();
        
        tabs_container->redraw();
    }

    void load_data(std::string filename) {
        std::ifstream file;
        file.open(filename);

        std::string line;
        std::vector<RowData> rows_data;

        if (file.is_open()) {
            while(std::getline(file, line)) {

                if (line.empty()) continue;

                std::stringstream sstream(line);

                RowData row;

                if (sstream >> row.name >> row.x >> row.y >> row.type >> row.date) {

                    row = set_distance(row);

                    row.group_name = set_group_name(row);

                    row.group_date = set_group_date(row);

                    rows_data.push_back(row);
                }
            }
        }

        file.close();

        if (rows_data.empty()) {
            fl_alert("Файл пуст или имеет неверный формат!");
            return;
        }

        set_group_type(rows_data);

        std::string label = filename.substr(filename.find_last_of("/\\") + 1);
        
        tabs_container->begin();
        Fl_Group* tab_page = new Fl_Group(tabs_container->x(), tabs_container->y() + 25, tabs_container->w(), tabs_container->h() - 25, strdup(label.c_str()));
        Data_Table* table = new Data_Table(tab_page->x() + 5, tab_page->y() + 5, tab_page->w() - 10, tab_page->h() - 10);
        table->add_data(rows_data);
        table->auto_size_columns();
        table->filaname = filename;
        tab_page->end();
        tabs_container->end();
        
        tabs_container->redraw();
    }

    std::string set_group_date(RowData row) {
        const int DAY_IN_SECONDS = 86400;
        const int TWO_DAYS_IN_SECONDS = 86400 * 2;
        const int WEEK_IN_SECONDS = 604800;
        const int MONTH_IN_SECONDS = 2629743;
        const int YEAR_IN_SECONDS = 31557600;
        const int TEN_YEAR_IN_SECONDS = 315576000;

        int current_date = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

        int row_date;
        try {
            row_date = std::stoi(row.date);
        }
        catch(const std::exception& e) {
            fl_alert("Неверный формат!");
            return "Неизвестно";
        }

        if ((current_date - row_date) < DAY_IN_SECONDS) {
            return "Сегодня";
        } else if ((current_date - row_date) < (TWO_DAYS_IN_SECONDS)) {
            return "Вчера";
        } else if ((current_date - row_date) < WEEK_IN_SECONDS) {
            return "На этой неделе";
        } else if ((current_date - row_date) < MONTH_IN_SECONDS) {
            return "В этом месяце";
        } else if ((current_date - row_date) < YEAR_IN_SECONDS) {
            return "В этом году";
        } else if ((current_date - row_date) < TEN_YEAR_IN_SECONDS) {
            return "В этом десятилетии";
        } 

        return "Раннее";
    }

    std::string set_group_name(RowData row) {
        if (row.name.empty()) return "#";

        unsigned char c1 = (unsigned char)row.name[0];
    
        if (c1 == 0xD0 || c1 == 0xD1) {
            if (row.name.size() >= 2) {
                unsigned char c2 = (unsigned char)row.name[1];
                std::string letter = row.name.substr(0, 2);
            
                if (c1 == 0xD0) {
                    if (c2 >= 0xB0 && c2 <= 0xBF) letter[1] = c2 - 0x20;
                } else if (c1 == 0xD1) {
                    if (c2 >= 0x80 && c2 <= 0x8F) {
                        letter[0] = 0xD0;
                        letter[1] = c2 + 0x20;
                    } else if (c2 == 0x91) { 
                        letter[0] = 0xD0;
                        letter[1] = 0x81;
                    }
                }

                std::string ABC = "ЙЦУКЕНГШЩЗХЪФЫВАПРОЛДЖЭЯЧСМИТЬБЮЁ";
                if (ABC.find(letter) != std::string::npos) {
                    return letter;
                }
            }
        }
    
        return "#";
    }

    RowData set_distance(RowData row) {
        long double x1 = 0;
        long double y1 = 0;
        long double x2, y2;
        try {
            x2 = std::stold(row.x);
            y2 = std::stold(row.y);
        }
        catch(const std::exception& e) {
            fl_alert("Неверный формат!");
            return row;
        }
        long double distance = std::sqrt(std::pow(x2 - x1, 2) + std::pow(y2 - y1, 2));
        row.distance = distance;

        if (distance < 100) {
            row.group_distance = "До 100 ед.";
        } else if (distance < 1000) {
            row.group_distance = "До 1000 ед.";
        } else if (distance < 10000) {
            row.group_distance = "До 10000 ед.";
        } else if (distance > 10000) {
            row.group_distance = "Слишком далеко.";
        }

        return row;
    }

    void set_group_type(std::vector<RowData> &rows_data) {

        int N = 5;

        std::unordered_map<std::string, int> types;

        for (int i = 0; i < rows_data.size(); i++) {

            std::string type = rows_data[i].type;
            auto pred = [type](const RowData &item) {
                return item.type == type;
            };
            if (std::find_if(rows_data.begin(), rows_data.end(), pred) != rows_data.end()) {
                types[rows_data[i].type]++;
            } else {
                types.insert({rows_data[i].type, 0});
            }

        }

        for (int i = 0; i < rows_data.size(); i++) {

            auto type = types.find(rows_data[i].type);

            rows_data[i].group_type = "Разное";

            if (type != types.end()) {
                if (types[rows_data[i].type] > N) {
                    rows_data[i].group_type = rows_data[i].type;
                }
            }
        }

    }

    bool is_grouped(void* data) {
        Main_Window* window = reinterpret_cast<Main_Window*>(data);
        Fl_Widget* current_tab = window->tabs_container->value();

        if (!current_tab) {
            fl_alert("Нет открытых вкладок!");
            return false;
        }

        Fl_Group* file_tab_page = reinterpret_cast<Fl_Group*>(current_tab);

        Fl_Tabs* nested_tabs = nullptr;
        if (file_tab_page->children() > 0) {
            nested_tabs = dynamic_cast<Fl_Tabs*>(file_tab_page->child(0));
        }

        if (!nested_tabs) {
            return false;
        }

        return true;
    }

    std::vector<Fl_Menu_Item> items = {
        {"&Файл", 0, nullptr, nullptr, FL_SUBMENU},
            {"&Новый", FL_COMMAND + 'n', &Main_Window::on_new_item_click, this},
            {"&Открыть", FL_COMMAND + 'o', &Main_Window::on_open_item_click, this},
            {"&Сохранить", FL_COMMAND + 's', &Main_Window::on_save_item_click, this},
            {"&Закрыть вкладку", FL_COMMAND + 'w', &Main_Window::on_close_item_click, this},
            {"&Выйти", FL_COMMAND + 'q', &Main_Window::on_exit_item_click, this},
        {0},
        {"&Группировать", 0, nullptr, nullptr, FL_SUBMENU},
            {"&По расстоянию", FL_COMMAND + 'r', &Main_Window::on_group_by_distance_item_click, this},
            {"&По имени", FL_COMMAND + 'i', &Main_Window::on_group_by_name_item_click, this},
            {"&По времени создания", FL_COMMAND + 't', &Main_Window::on_group_by_date_item_click, this},
            {"&По типу", FL_COMMAND + 'y', &Main_Window::on_group_by_type_item_click, this},
            {"&Разгруппировать", FL_COMMAND + 'd', &Main_Window::on_ungroup_item_click, this},
        {0},
        {"&Правка", 0, nullptr, nullptr, FL_SUBMENU},
            {"&Добавить объект", FL_COMMAND + 'a', &Main_Window::on_add_item_click, this},
        {0},
        {0}
    };

    Fl_Menu_Bar menu_bar {0, 0, 850, 30, nullptr};
    Fl_Tabs* tabs_container;
};
 
int main(int argc, char** argv) {

    Main_Window *window = new Main_Window();
    window->end();
    window->show(argc, argv);

    return Fl::run();
}