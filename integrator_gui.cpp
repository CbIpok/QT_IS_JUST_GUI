#if defined(_WIN32)
#include <windows.h>
#endif

#if defined(_WIN32) && !defined(WIN32)
#define WIN32
#endif

#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Native_File_Chooser.H>
#include <FL/Fl_Text_Buffer.H>
#include <FL/Fl_Text_Display.H>
#include <FL/Fl_Table_Row.H>
#include <FL/fl_ask.H>
#include <FL/fl_draw.H>

#include <optional>
#include <sstream>
#include <string>

#include "data_integrator.hpp"

namespace {

class DriverTableView : public Fl_Table_Row {
public:
    DriverTableView(int X, int Y, int W, int H, DataIntegrator& integrator)
        : Fl_Table_Row(X, Y, W, H), integrator_(integrator) {
        row_header(1);
        row_header_width(40);
        col_header(1);
        col_header_height(26);
        cols(3);
        col_width(0, 180);
        col_width(1, 240);
        col_width(2, 160);
        end();
    }

    void refresh() {
        rows(static_cast<int>(integrator_.driverCount()));
        redraw();
    }

protected:
    void draw_cell(TableContext context, int row, int col, int x, int y, int w, int h) override {
        switch (context) {
            case CONTEXT_ROW_HEADER: {
                fl_push_clip(x, y, w, h);
                fl_draw_box(FL_THIN_UP_BOX, x, y, w, h, color());
                fl_color(FL_BLACK);
                std::string label = std::to_string(row + 1);
                fl_draw(label.c_str(), x + 6, y + h - 6);
                fl_pop_clip();
                break;
            }
            case CONTEXT_COL_HEADER: {
                static const char* headers[] = {"Лицензия", "ФИО", "Авто"};
                fl_push_clip(x, y, w, h);
                fl_draw_box(FL_THIN_UP_BOX, x, y, w, h, color());
                fl_color(FL_BLACK);
                fl_draw(headers[col], x + 6, y + h - 6);
                fl_pop_clip();
                break;
            }
            case CONTEXT_CELL: {
                fl_push_clip(x, y, w, h);
                fl_draw_box(FL_FLAT_BOX, x, y, w, h, FL_WHITE);
                fl_color(FL_BLACK);
                auto recordOpt = integrator_.driverAt(static_cast<std::size_t>(row));
                if (recordOpt.has_value()) {
                    const DriverRecord& driver = recordOpt.value();
                    std::string text;
                    switch (col) {
                        case 0: text = driver.licenseNumber; break;
                        case 1: text = driver.fio; break;
                        case 2: text = driver.carBrand; break;
                        default: break;
                    }
                    fl_draw(text.c_str(), x + 4, y + h - 6);
                }
                fl_pop_clip();
                break;
            }
            case CONTEXT_RC_RESIZE: {
                int totalWidth = w > 0 ? w : this->w();
                col_width(0, totalWidth * 0.30);
                col_width(1, totalWidth * 0.45);
                col_width(2, totalWidth * 0.25);
                break;
            }
            default:
                break;
        }
    }

private:
    DataIntegrator& integrator_;
};

class OrderTableView : public Fl_Table_Row {
public:
    OrderTableView(int X, int Y, int W, int H, DataIntegrator& integrator)
        : Fl_Table_Row(X, Y, W, H), integrator_(integrator) {
        row_header(1);
        row_header_width(40);
        col_header(1);
        col_header_height(26);
        cols(4);
        col_width(0, 140);
        col_width(1, 220);
        col_width(2, 120);
        col_width(3, 140);
        end();
    }

    void refresh() {
        rows(static_cast<int>(integrator_.orderCount()));
        redraw();
    }

protected:
    void draw_cell(TableContext context, int row, int col, int x, int y, int w, int h) override {
        switch (context) {
            case CONTEXT_ROW_HEADER: {
                fl_push_clip(x, y, w, h);
                fl_draw_box(FL_THIN_UP_BOX, x, y, w, h, color());
                fl_color(FL_BLACK);
                std::string label = std::to_string(row + 1);
                fl_draw(label.c_str(), x + 6, y + h - 6);
                fl_pop_clip();
                break;
            }
            case CONTEXT_COL_HEADER: {
                static const char* headers[] = {"Лицензия", "Адрес", "Цена", "Дата"};
                fl_push_clip(x, y, w, h);
                fl_draw_box(FL_THIN_UP_BOX, x, y, w, h, color());
                fl_color(FL_BLACK);
                fl_draw(headers[col], x + 6, y + h - 6);
                fl_pop_clip();
                break;
            }
            case CONTEXT_CELL: {
                fl_push_clip(x, y, w, h);
                fl_draw_box(FL_FLAT_BOX, x, y, w, h, FL_WHITE);
                fl_color(FL_BLACK);
                auto recordOpt = integrator_.orderAt(static_cast<std::size_t>(row));
                if (recordOpt.has_value()) {
                    const OrderRecord& order = recordOpt.value();
                    std::string text;
                    switch (col) {
                        case 0: text = order.licenseNumber; break;
                        case 1: text = order.address; break;
                        case 2: text = order.cost; break;
                        case 3: text = order.date.displayString(); break;
                        default: break;
                    }
                    fl_draw(text.c_str(), x + 4, y + h - 6);
                }
                fl_pop_clip();
                break;
            }
            case CONTEXT_RC_RESIZE: {
                int totalWidth = w > 0 ? w : this->w();
                col_width(0, totalWidth * 0.24);
                col_width(1, totalWidth * 0.36);
                col_width(2, totalWidth * 0.18);
                col_width(3, totalWidth * 0.22);
                break;
            }
            default:
                break;
        }
    }

private:
    DataIntegrator& integrator_;
};

class IntegratorGUI {
public:
    IntegratorGUI();
    ~IntegratorGUI();

    void show();

private:
    DataIntegrator integrator_;

    Fl_Double_Window* hashWindow_;
    Fl_Double_Window* treeWindow_;
    Fl_Menu_Bar*      hashMenuBar_;
    Fl_Menu_Bar*      treeMenuBar_;
    DriverTableView*  driverTable_;
    OrderTableView*   orderTable_;
    Fl_Button*        hashDebugButton_;
    Fl_Button*        treeDebugButton_;
    Fl_Button*        dateDebugButton_;
    Fl_Box*           hashStatusBox_;
    Fl_Box*           treeStatusBox_;

    void refreshDataViews();
    void updateStatus(const std::string& message);
    void showInfo(const std::string& message);
    void showError(const std::string& message);
    void showTextWindow(const std::string& title, const std::string& content);

    std::optional<std::string> promptNonEmpty(const char* prompt, const std::string& initial = "");
    std::optional<std::string> promptString(const char* prompt, const std::string& initial = "");
    std::optional<int>         promptInt(const char* prompt, int initial, int minimum);
    std::optional<DriverRecord> promptDriver(const DriverRecord* initial = nullptr);
    std::optional<OrderRecord>  promptOrder(const OrderRecord* initial = nullptr);
    std::optional<std::size_t>  promptIndexSelection(std::size_t count, const std::string& title);
    std::optional<std::string>  promptFilePath(const char* dialogTitle,
                                               const char* manualPrompt,
                                               Fl_Native_File_Chooser::Type type,
                                               bool allowEmpty = false);

    void handleLoadDrivers();
    void handleLoadOrders();
    void handleSave();
    void handleSaveStructures();
    void handleClear();
    void handleCreateDriverTable();
    void handleClearDriverTableOnly();
    void handleShowDriverTable();

    void handleAddDriver();
    void handleUpdateDriver();
    void handleRemoveDriver();
    void handleFindDriver();

    void handleAddOrder();
    void handleUpdateOrder();
    void handleRemoveOrder();
    void handleCheckOrder();
    void handleCreateOrderTree();
    void handleClearOrderTreeOnly();
    void handleShowOrderTree();
    void handleShowDateTree();
    void handleGenerateReport();

    static void CallbackLoadDrivers(Fl_Widget*, void*);
    static void CallbackLoadOrders(Fl_Widget*, void*);
    static void CallbackSave(Fl_Widget*, void*);
    static void CallbackSaveStructures(Fl_Widget*, void*);
    static void CallbackClear(Fl_Widget*, void*);
    static void CallbackCreateDriverTable(Fl_Widget*, void*);
    static void CallbackClearDriverTableOnly(Fl_Widget*, void*);
    static void CallbackShowDriverTable(Fl_Widget*, void*);
    static void CallbackAddDriver(Fl_Widget*, void*);
    static void CallbackUpdateDriver(Fl_Widget*, void*);
    static void CallbackRemoveDriver(Fl_Widget*, void*);
    static void CallbackFindDriver(Fl_Widget*, void*);
    static void CallbackAddOrder(Fl_Widget*, void*);
    static void CallbackUpdateOrder(Fl_Widget*, void*);
    static void CallbackRemoveOrder(Fl_Widget*, void*);
    static void CallbackCheckOrder(Fl_Widget*, void*);
    static void CallbackCreateOrderTree(Fl_Widget*, void*);
    static void CallbackClearOrderTreeOnly(Fl_Widget*, void*);
    static void CallbackShowOrderTree(Fl_Widget*, void*);
    static void CallbackShowDateTree(Fl_Widget*, void*);
    static void CallbackGenerateReport(Fl_Widget*, void*);
};

IntegratorGUI::IntegratorGUI()
    : integrator_(),
      hashWindow_(nullptr),
      treeWindow_(nullptr),
      hashMenuBar_(nullptr),
      treeMenuBar_(nullptr),
      driverTable_(nullptr),
      orderTable_(nullptr),
      hashDebugButton_(nullptr),
      treeDebugButton_(nullptr),
      dateDebugButton_(nullptr),
      hashStatusBox_(nullptr),
      treeStatusBox_(nullptr) {
    const int windowWidth = 700;
    const int windowHeight = 700;

    hashWindow_ = new Fl_Double_Window(windowWidth, windowHeight, "Водители (хеш-таблица)");
    hashWindow_->begin();

    const int menuBarHeight = 32;
    hashMenuBar_ = new Fl_Menu_Bar(0, 0, windowWidth, menuBarHeight);
    hashMenuBar_->box(FL_THIN_UP_BOX);
    hashMenuBar_->color(fl_rgb_color(245, 245, 245));
    hashMenuBar_->textsize(13);
    hashMenuBar_->add("Загр. вод", 0, &IntegratorGUI::CallbackLoadDrivers, this);
    hashMenuBar_->add("Выгр. в файл", 0, &IntegratorGUI::CallbackSave, this);
    hashMenuBar_->add("Доб", 0, &IntegratorGUI::CallbackAddDriver, this);
    hashMenuBar_->add("Изм", 0, &IntegratorGUI::CallbackUpdateDriver, this);
    hashMenuBar_->add("Найти", 0, &IntegratorGUI::CallbackFindDriver, this);
    hashMenuBar_->add("Удалить", 0, &IntegratorGUI::CallbackRemoveDriver, this);
    hashMenuBar_->add("Отч", 0, &IntegratorGUI::CallbackSaveStructures, this);
    hashMenuBar_->add("Табл", 0, &IntegratorGUI::CallbackShowDriverTable, this);
    hashMenuBar_->add("Созд Табл", 0, &IntegratorGUI::CallbackCreateDriverTable, this);
    hashMenuBar_->add("Удалить Табл", 0, &IntegratorGUI::CallbackClearDriverTableOnly, this);
    hashMenuBar_->add("Очистить", 0, &IntegratorGUI::CallbackClear, this);

    hashStatusBox_ = new Fl_Box(10, menuBarHeight + 5, windowWidth - 20, 30);
    hashStatusBox_->box(FL_THIN_DOWN_BOX);
    hashStatusBox_->labelfont(FL_HELVETICA_BOLD);
    hashStatusBox_->labelsize(14);
    hashStatusBox_->align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT);
    hashStatusBox_->copy_label("Готово.");

    const int hashContentTop = menuBarHeight + 40;
    const int hashContentHeight = windowHeight - hashContentTop - 10;

    Fl_Box* hashLabel = new Fl_Box(10, hashContentTop, windowWidth - 20, 25, "Хеш-таблица водителей");
    hashLabel->labelfont(FL_HELVETICA_BOLD);
    hashLabel->labelsize(14);
    hashLabel->align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT);

    const int driverTableTop = hashContentTop + 25;
    const int driverButtonHeight = 30;
    const int driverButtonGap = 10;
    const int driverTableHeight = hashContentHeight - 25 - driverButtonHeight - driverButtonGap;

    driverTable_ = new DriverTableView(10, driverTableTop, windowWidth - 20, driverTableHeight, integrator_);

    int driverButtonsTop = driverTableTop + driverTableHeight + driverButtonGap;
    hashDebugButton_ = new Fl_Button(10,
                                     driverButtonsTop,
                                     200,
                                     driverButtonHeight,
                                     "Отладка хеш-таблицы");
    hashDebugButton_->callback(&IntegratorGUI::CallbackShowDriverTable, this);

    hashWindow_->end();
    hashWindow_->resizable(driverTable_);

    treeWindow_ = new Fl_Double_Window(windowWidth, windowHeight, "Заказы (AVL-дерево)");
    treeWindow_->begin();

    treeMenuBar_ = new Fl_Menu_Bar(0, 0, windowWidth, menuBarHeight);
    treeMenuBar_->box(FL_THIN_UP_BOX);
    treeMenuBar_->color(fl_rgb_color(245, 245, 245));
    treeMenuBar_->textsize(13);
    treeMenuBar_->add("Загр. зак", 0, &IntegratorGUI::CallbackLoadOrders, this);
    treeMenuBar_->add("Доб", 0, &IntegratorGUI::CallbackAddOrder, this);
    treeMenuBar_->add("Изм", 0, &IntegratorGUI::CallbackUpdateOrder, this);
    treeMenuBar_->add("Найти", 0, &IntegratorGUI::CallbackCheckOrder, this);
    treeMenuBar_->add("Удалить", 0, &IntegratorGUI::CallbackRemoveOrder, this);
    treeMenuBar_->add("Табл", 0, &IntegratorGUI::CallbackShowOrderTree, this);
    treeMenuBar_->add("Табл дат", 0, &IntegratorGUI::CallbackShowDateTree, this);
    treeMenuBar_->add("Созд Табл", 0, &IntegratorGUI::CallbackCreateOrderTree, this);
    treeMenuBar_->add("Удалить Табл", 0, &IntegratorGUI::CallbackClearOrderTreeOnly, this);
    treeMenuBar_->add("Заказы (Генерация отчётов)", 0, &IntegratorGUI::CallbackGenerateReport, this);

    treeStatusBox_ = new Fl_Box(10, menuBarHeight + 5, windowWidth - 20, 30);
    treeStatusBox_->box(FL_THIN_DOWN_BOX);
    treeStatusBox_->labelfont(FL_HELVETICA_BOLD);
    treeStatusBox_->labelsize(14);
    treeStatusBox_->align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT);
    treeStatusBox_->copy_label("Готово.");

    const int treeContentTop = menuBarHeight + 40;
    const int treeContentHeight = windowHeight - treeContentTop - 10;

    Fl_Box* treeLabel = new Fl_Box(10, treeContentTop, windowWidth - 20, 25, "Дерево заказов (AVL)");
    treeLabel->labelfont(FL_HELVETICA_BOLD);
    treeLabel->labelsize(14);
    treeLabel->align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT);

    const int orderTableTop = treeContentTop + 25;
    const int orderButtonHeight = 30;
    const int orderButtonGap = 10;
    const int orderTableHeight = treeContentHeight - 25 - orderButtonHeight - orderButtonGap;

    orderTable_ = new OrderTableView(10, orderTableTop, windowWidth - 20, orderTableHeight, integrator_);

    int orderButtonsTop = orderTableTop + orderTableHeight + orderButtonGap;
    treeDebugButton_ = new Fl_Button(10,
                                     orderButtonsTop,
                                     200,
                                     orderButtonHeight,
                                     "Отладка AVL (водители)");
    treeDebugButton_->callback(&IntegratorGUI::CallbackShowOrderTree, this);

    dateDebugButton_ = new Fl_Button(220,
                                     orderButtonsTop,
                                     240,
                                     orderButtonHeight,
                                     "Отладка AVL (даты)");
    dateDebugButton_->callback(&IntegratorGUI::CallbackShowDateTree, this);

    treeWindow_->end();
    treeWindow_->resizable(orderTable_);
}

IntegratorGUI::~IntegratorGUI() {
    delete hashWindow_;
    delete treeWindow_;
}

void IntegratorGUI::show() {
    if (hashWindow_) hashWindow_->show();
    if (treeWindow_) treeWindow_->show();
    refreshDataViews();
}

void IntegratorGUI::refreshDataViews() {
    std::ostringstream status;
    status << "Водителей: " << integrator_.driverCount()
           << " | Заказов: " << integrator_.orderCount();
    if (hashStatusBox_) hashStatusBox_->copy_label(status.str().c_str());
    if (treeStatusBox_) treeStatusBox_->copy_label(status.str().c_str());

    if (driverTable_) {
        driverTable_->refresh();
    }
    if (orderTable_) {
        orderTable_->refresh();
    }

    if (hashDebugButton_) {
        if (integrator_.hasDriverTable()) hashDebugButton_->activate();
        else hashDebugButton_->deactivate();
    }
    if (treeDebugButton_) {
        if (integrator_.hasOrderTree()) treeDebugButton_->activate();
        else treeDebugButton_->deactivate();
    }
    if (dateDebugButton_) {
        if (integrator_.hasOrderTree()) dateDebugButton_->activate();
        else dateDebugButton_->deactivate();
    }
}

void IntegratorGUI::updateStatus(const std::string& message) {
    if (hashStatusBox_) hashStatusBox_->copy_label(message.c_str());
    if (treeStatusBox_) treeStatusBox_->copy_label(message.c_str());
}

void IntegratorGUI::showInfo(const std::string& message) {
    fl_message_title("Информация");
    fl_message("%s", message.c_str());
}

void IntegratorGUI::showError(const std::string& message) {
    fl_message_title("Ошибка");
    fl_alert("%s", message.c_str());
}

void IntegratorGUI::showTextWindow(const std::string& title, const std::string& content) {
    Fl_Double_Window* window = new Fl_Double_Window(640, 480, title.c_str());
    window->begin();
    Fl_Text_Display* display = new Fl_Text_Display(10, 10, 620, 430);
    display->box(FL_DOWN_BOX);
    display->textfont(FL_COURIER);
    display->textsize(13);
    Fl_Text_Buffer* buffer = new Fl_Text_Buffer();
    buffer->text(content.c_str());
    display->buffer(buffer);
    window->resizable(display);
    window->end();
    struct TextWindowContext {
        Fl_Text_Display* display;
        Fl_Text_Buffer*  buffer;
    };
    auto* context = new TextWindowContext{display, buffer};
    window->callback([](Fl_Widget* widget, void* data) {
        auto* ctx = static_cast<TextWindowContext*>(data);
        if (ctx) {
            if (ctx->display) {
                ctx->display->buffer(nullptr);
            }
            delete ctx->buffer;
            delete ctx;
        }
        delete widget;
    }, context);
    window->set_non_modal();
    window->show();
}

std::optional<std::string> IntegratorGUI::promptNonEmpty(const char* prompt, const std::string& initial) {
    std::string current = initial;
    while (true) {
        const char* value = fl_input("%s", current.c_str(), prompt);
        if (!value) {
            return std::nullopt;
        }
        std::string result = value;
        if (result.empty()) {
            showError("Поле не может быть пустым.");
            current = result;
            continue;
        }
        return result;
    }
}

std::optional<std::string> IntegratorGUI::promptString(const char* prompt, const std::string& initial) {
    std::string current = initial;
    const char* value = fl_input("%s", current.c_str(), prompt);
    if (!value) {
        return std::nullopt;
    }
    return std::string(value);
}

std::optional<int> IntegratorGUI::promptInt(const char* prompt, int initial, int minimum) {
    std::string current = std::to_string(initial);
    while (true) {
        const char* value = fl_input("%s", current.c_str(), prompt);
        if (!value) {
            return std::nullopt;
        }
        std::string result = value;
        if (result.empty()) {
            showError("Введите число.");
            current = result;
            continue;
        }
        try {
            int parsed = std::stoi(result);
            if (parsed < minimum) {
                std::ostringstream error;
                error << "Значение должно быть не меньше " << minimum << ".";
                showError(error.str());
                current = result;
                continue;
            }
            return parsed;
        }
        catch (...) {
            showError("Введите корректное целое число.");
            current = result;
        }
    }
}

std::optional<DriverRecord> IntegratorGUI::promptDriver(const DriverRecord* initial) {
    std::optional<std::string> license = promptNonEmpty("Номер водительского удостоверения:", initial ? initial->licenseNumber : "");
    if (!license) return std::nullopt;

    std::optional<std::string> fio = promptNonEmpty("ФИО водителя:", initial ? initial->fio : "");
    if (!fio) return std::nullopt;

    std::optional<std::string> carBrand = promptNonEmpty("Марка автомобиля:", initial ? initial->carBrand : "");
    if (!carBrand) return std::nullopt;

    DriverRecord record{*license, *fio, *carBrand};
    return record;
}

std::optional<OrderRecord> IntegratorGUI::promptOrder(const OrderRecord* initial) {
    std::optional<std::string> license = promptNonEmpty("Номер водителя (лицензия):", initial ? initial->licenseNumber : "");
    if (!license) return std::nullopt;

    std::optional<std::string> address = promptNonEmpty("Адрес заказа:", initial ? initial->address : "");
    if (!address) return std::nullopt;

    std::optional<std::string> cost = promptNonEmpty("Стоимость:", initial ? initial->cost : "");
    if (!cost) return std::nullopt;

    std::string datePromptDefault;
    if (initial && initial->date.isValid()) {
        datePromptDefault = initial->date.displayString();
    }

    std::string currentDate = datePromptDefault;
    Date parsedDate{};
    while (true) {
        std::optional<std::string> dateInput = promptNonEmpty("Дата (YYYY-MM-DD или DD Mon YYYY):", currentDate);
        if (!dateInput) {
            return std::nullopt;
        }
        if (Date::parse(*dateInput, parsedDate)) {
            break;
        }
        showError("Введите корректную дату в формате YYYY-MM-DD или DD Mon YYYY.");
        currentDate = *dateInput;
    }

    OrderRecord record{*license, *address, *cost, parsedDate};
    return record;
}

std::optional<std::size_t> IntegratorGUI::promptIndexSelection(std::size_t count, const std::string& title) {
    if (count == 0) {
        return std::nullopt;
    }

    std::string current = "1";
    while (true) {
        const char* input = fl_input("%s", current.c_str(), title.c_str());
        if (!input) {
            return std::nullopt;
        }
        std::string value = input;
        if (value.empty()) {
            showError("Введите номер записи.");
            current = value;
            continue;
        }
        try {
            long parsed = std::stol(value);
            if (parsed < 1 || parsed > static_cast<long>(count)) {
                std::ostringstream message;
                message << "Введите число от 1 до " << count << ".";
                showError(message.str());
                current = value;
                continue;
            }
            return static_cast<std::size_t>(parsed - 1);
        }
        catch (...) {
            showError("Введите корректное число.");
            current = value;
        }
    }
}

std::optional<std::string> IntegratorGUI::promptFilePath(const char* dialogTitle,
                                                         const char* manualPrompt,
                                                         Fl_Native_File_Chooser::Type type,
                                                         bool allowEmpty) {
    Fl_Native_File_Chooser chooser(type);
    chooser.title(dialogTitle);
    if (type == Fl_Native_File_Chooser::BROWSE_SAVE_FILE) {
        chooser.options(Fl_Native_File_Chooser::SAVEAS_CONFIRM);
    }

    int result = chooser.show();
    if (result == 0) {
        const char* filename = chooser.filename();
        if (filename && *filename) {
            return std::string(filename);
        }
        return allowEmpty ? std::optional<std::string>(std::string()) : std::nullopt;
    }
    if (result == 1) {
        if (allowEmpty) {
            return promptString(manualPrompt);
        }
        return promptNonEmpty(manualPrompt);
    }

    std::ostringstream error;
    error << "Ошибка выбора файла: " << chooser.errmsg();
    showError(error.str());
    return std::nullopt;
}

void IntegratorGUI::handleLoadDrivers() {
    auto path = promptFilePath("Выбор файла водителей",
                               "Введите путь к файлу водителей:",
                               Fl_Native_File_Chooser::BROWSE_FILE,
                               false);
    if (!path) return;

    std::size_t requestedCapacity = 0;
    if (!integrator_.hasDriverTable()) {
        std::size_t currentCapacity = integrator_.driverTableCapacity();
        if (currentCapacity == 0) {
            currentCapacity = 1;
        }
        int defaultSize = static_cast<int>(currentCapacity);
        auto sizeOpt = promptInt("Начальный размер хеш-таблицы:", defaultSize, 1);
        if (!sizeOpt) return;
        requestedCapacity = static_cast<std::size_t>(*sizeOpt);
    }

    std::size_t previousOrderCount = integrator_.orderCount();

    if (integrator_.loadDriversFromFile(*path, requestedCapacity)) {
        std::size_t newOrderCount = integrator_.orderCount();
        std::string  status = (newOrderCount > previousOrderCount)
                                 ? "Файл водителей и заказов загружен."
                                 : "Файл водителей загружен.";
        updateStatus(status);
        refreshDataViews();
        showInfo(status);
    }
    else {
        showError("Не удалось загрузить файл водителей.");
    }
}

void IntegratorGUI::handleLoadOrders() {
    if (!integrator_.hasDriverTable()) {
        showError("Сначала загрузите или создайте хеш-таблицу водителей.");
        return;
    }

    auto path = promptFilePath("Выбор файла заказов",
                               "Введите путь к файлу заказов:",
                               Fl_Native_File_Chooser::BROWSE_FILE,
                               false);
    if (!path) return;

    if (integrator_.loadOrdersFromFile(*path)) {
        updateStatus("Файл заказов загружен.");
        refreshDataViews();
        showInfo("Заказы загружены.");
    }
    else {
        showError("Не удалось загрузить файл заказов. Проверьте данные и наличие водителей.");
    }
}

void IntegratorGUI::handleSave() {
    auto path = promptFilePath("Сохранение конфигурации",
                               "Введите путь для сохранения конфигурации:",
                               Fl_Native_File_Chooser::BROWSE_SAVE_FILE,
                               false);
    if (!path) return;

    if (integrator_.saveToFile(*path)) {
        updateStatus("Данные сохранены.");
        showInfo("Файл сохранён.");
    }
    else {
        showError("Не удалось сохранить файл.");
    }
}

void IntegratorGUI::handleSaveStructures() {
    auto hashPath = promptFilePath("Сохранение хеш-таблицы",
                                   "Файл для хеш-таблицы (оставьте пустым, чтобы пропустить):",
                                   Fl_Native_File_Chooser::BROWSE_SAVE_FILE,
                                   true);
    if (!hashPath) return;
    auto treePath = promptFilePath("Сохранение дерева заказов",
                                   "Файл для дерева заказов (оставьте пустым, чтобы пропустить):",
                                   Fl_Native_File_Chooser::BROWSE_SAVE_FILE,
                                   true);
    if (!treePath) return;

    if (integrator_.saveStructures(*hashPath, *treePath)) {
        updateStatus("Структуры сохранены.");
        showInfo("Хеш-таблица и дерево сохранены.");
    }
    else {
        showError("Не удалось сохранить структуры.");
    }
}

void IntegratorGUI::handleClear() {
    int choice = fl_choice("Очистить все данные интегратора?", "Отмена", "Очистить", nullptr);
    if (choice == 1) {
        integrator_.clear();
        refreshDataViews();
        updateStatus("Интегратор очищен.");
    }
}

void IntegratorGUI::handleCreateDriverTable() {
    int defaultSize = static_cast<int>(integrator_.driverTableCapacity());
    auto sizeOpt = promptInt("Начальный размер хеш-таблицы:", defaultSize, 1);
    if (!sizeOpt) return;

    if (integrator_.createDriverTable(static_cast<std::size_t>(*sizeOpt))) {
        refreshDataViews();
        updateStatus("Хеш-таблица создана.");
    }
    else {
        showError("Не удалось создать хеш-таблицу.");
    }
}

void IntegratorGUI::handleClearDriverTableOnly() {
    if (!integrator_.hasDriverTable()) {
        showError("Хеш-таблица ещё не создана.");
        return;
    }
    int choice = fl_choice("Удалить хеш-таблицу водителей?", "Отмена", "Удалить", nullptr);
    if (choice == 1) {
        integrator_.clearDriverTable();
        refreshDataViews();
        updateStatus("Хеш-таблица удалена.");
    }
}

void IntegratorGUI::handleShowDriverTable() {
    if (!integrator_.hasDriverTable()) {
        showError("Хеш-таблица ещё не создана.");
        return;
    }
    std::string text = integrator_.hashTableAsText();
    if (text.empty()) {
        text = "<пусто>";
    }
    showTextWindow("Хеш-таблица водителей", text);
}

void IntegratorGUI::handleAddDriver() {
    if (!integrator_.hasDriverTable()) {
        showError("Хеш-таблица ещё не создана. Создайте таблицу перед добавлением водителей.");
        return;
    }
    auto record = promptDriver();
    if (!record) return;

    if (integrator_.addDriver(*record)) {
        refreshDataViews();
        updateStatus("Водитель добавлен.");
    }
    else {
        showError("Не удалось добавить водителя. Возможно, лицензия уже существует или данные некорректны.");
    }
}

void IntegratorGUI::handleUpdateDriver() {
    if (!integrator_.hasDriverTable()) {
        showError("Хеш-таблица ещё не создана. Создайте таблицу перед изменением водителей.");
        return;
    }
    auto license = promptNonEmpty("Номер водителя для изменения:");
    if (!license) return;

    auto stored = integrator_.findDriver(*license);
    if (!stored.has_value()) {
        showError("Водитель с таким номером не найден.");
        return;
    }

    auto updated = promptDriver(&stored.value());
    if (!updated) return;

    if (updated->licenseNumber != stored->licenseNumber) {
        showError("Нельзя изменять номер водительского удостоверения.");
        return;
    }

    if (integrator_.updateDriver(*stored, *updated)) {
        refreshDataViews();
        updateStatus("Данные водителя обновлены.");
    }
    else {
        showError("Не удалось обновить данные водителя.");
    }
}

void IntegratorGUI::handleRemoveDriver() {
    if (!integrator_.hasDriverTable()) {
        showError("Хеш-таблица ещё не создана. Создайте таблицу перед удалением водителей.");
        return;
    }
    auto license = promptNonEmpty("Номер водителя для удаления:");
    if (!license) return;

    int choice = fl_choice("Удалить водителя и связанные заказы?", "Отмена", "Удалить", nullptr);
    if (choice != 1) {
        return;
    }

    if (integrator_.removeDriver(*license)) {
        refreshDataViews();
        updateStatus("Водитель удалён.");
    }
    else {
        showError("Не удалось удалить водителя.");
    }
}

void IntegratorGUI::handleFindDriver() {
    if (!integrator_.hasDriverTable()) {
        showError("Хеш-таблица ещё не создана.");
        return;
    }
    auto license = promptNonEmpty("Номер водителя для поиска:");
    if (!license) return;

    auto stored = integrator_.findDriver(*license);
    if (!stored.has_value()) {
        showError("Водитель не найден.");
        return;
    }

    std::ostringstream info;
    info << "Лицензия: " << stored->licenseNumber << "\n"
         << "ФИО: " << stored->fio << "\n"
         << "Марка: " << stored->carBrand;
    showInfo(info.str());
}

void IntegratorGUI::handleAddOrder() {
    if (!integrator_.hasDriverTable() || !integrator_.hasOrderTree()) {
        showError("Создайте хеш-таблицу и дерево заказов перед добавлением заказов.");
        return;
    }
    auto record = promptOrder();
    if (!record) return;

    if (integrator_.addOrder(*record)) {
        refreshDataViews();
        updateStatus("Заказ добавлен.");
    }
    else {
        showError("Не удалось добавить заказ. Проверьте существование водителя и корректность данных.");
    }
}

void IntegratorGUI::handleUpdateOrder() {
    if (!integrator_.hasOrderTree()) {
        showError("Дерево заказов ещё не создано.");
        return;
    }
    if (!integrator_.hasDriverTable()) {
        showError("Хеш-таблица ещё не создана.");
        return;
    }
    auto license = promptNonEmpty("Номер водителя для выбора заказа:");
    if (!license) return;

    DoublyLinkedList<OrderRecord> orders = integrator_.ordersForDriver(*license);
    if (orders.empty()) {
        showError("Для выбранного водителя нет заказов.");
        return;
    }

    std::ostringstream list;
    list << "Заказы водителя " << *license << ":\n\n";
    for (std::size_t i = 0; i < orders.size(); ++i) {
        const OrderRecord& order = orders[i];
        list << (i + 1) << ") " << order.address << " | " << order.cost << " | " << order.date.displayString() << "\n";
    }
    showInfo(list.str());

    auto index = promptIndexSelection(orders.size(), "Номер заказа для изменения:");
    if (!index) return;

    OrderRecord original = orders[*index];
    auto updated = promptOrder(&original);
    if (!updated) return;

    if (integrator_.updateOrder(original, *updated)) {
        refreshDataViews();
        updateStatus("Заказ обновлён.");
    }
    else {
        showError("Не удалось обновить заказ. Проверьте данные и существование водителя.");
    }
}

void IntegratorGUI::handleRemoveOrder() {
    if (!integrator_.hasOrderTree()) {
        showError("Дерево заказов ещё не создано.");
        return;
    }
    auto license = promptNonEmpty("Номер водителя для удаления заказа:");
    if (!license) return;

    DoublyLinkedList<OrderRecord> orders = integrator_.ordersForDriver(*license);
    if (orders.empty()) {
        showError("Для выбранного водителя нет заказов.");
        return;
    }

    std::ostringstream list;
    list << "Заказы водителя " << *license << ":\n\n";
    for (std::size_t i = 0; i < orders.size(); ++i) {
        const OrderRecord& order = orders[i];
        list << (i + 1) << ") " << order.address << " | " << order.cost << " | " << order.date.displayString() << "\n";
    }
    showInfo(list.str());

    auto index = promptIndexSelection(orders.size(), "Номер заказа для удаления:");
    if (!index) return;

    OrderRecord target = orders[*index];
    int choice = fl_choice("Удалить выбранный заказ?", "Отмена", "Удалить", nullptr);
    if (choice != 1) {
        return;
    }

    if (integrator_.removeOrder(target)) {
        refreshDataViews();
        updateStatus("Заказ удалён.");
    }
    else {
        showError("Не удалось удалить заказ.");
    }
}

void IntegratorGUI::handleCheckOrder() {
    if (!integrator_.hasOrderTree()) {
        showError("Дерево заказов ещё не создано.");
        return;
    }
    auto record = promptOrder();
    if (!record) return;

    if (integrator_.hasOrder(*record)) {
        showInfo("Такой заказ найден в системе.");
    }
    else {
        showInfo("Такого заказа нет.");
    }
}

void IntegratorGUI::handleCreateOrderTree() {
    if (integrator_.createOrderTree()) {
        refreshDataViews();
        updateStatus("Дерево заказов создано.");
    }
    else {
        showError("Не удалось создать дерево заказов.");
    }
}

void IntegratorGUI::handleClearOrderTreeOnly() {
    if (!integrator_.hasOrderTree()) {
        showError("Дерево заказов ещё не создано.");
        return;
    }
    int choice = fl_choice("Удалить дерево заказов?", "Отмена", "Удалить", nullptr);
    if (choice == 1) {
        integrator_.clearOrderTree();
        refreshDataViews();
        updateStatus("Дерево заказов удалено.");
    }
}

void IntegratorGUI::handleShowOrderTree() {
    if (!integrator_.hasOrderTree()) {
        showError("Дерево заказов ещё не создано.");
        return;
    }
    std::string text = integrator_.orderTreeAsText();
    if (text.empty()) {
        text = "<пусто>";
    }
    showTextWindow("Дерево заказов", text);
}

void IntegratorGUI::handleShowDateTree() {
    if (!integrator_.hasOrderTree()) {
        showError("Дерево заказов ещё не создано.");
        return;
    }
    std::string text = integrator_.orderDateTreeAsText();
    if (text.empty()) {
        text = "<пусто>";
    }
    showTextWindow("Дерево заказов по датам", text);
}

void IntegratorGUI::handleGenerateReport() {
    if (!integrator_.hasDriverTable() || !integrator_.hasOrderTree()) {
        showError("Создайте обе структуры данных перед формированием отчёта.");
        return;
    }

    auto license = promptString("Номер лицензии для отчёта (можно оставить пустым):", "");
    if (!license) return;
    auto carBrand = promptString("Марка автомобиля (можно оставить пустым):", "");
    if (!carBrand) return;
    auto address = promptString("Адрес заказа (можно оставить пустым):", "");
    if (!address) return;
    auto fromDate = promptString("Начальная дата (включительно, можно оставить пустым):", "");
    if (!fromDate) return;
    auto toDate = promptString("Конечная дата (включительно, можно оставить пустым):", "");
    if (!toDate) return;

    DoublyLinkedList<ReportEntry> entries = integrator_.generateReport(*license, *carBrand, *address, *fromDate, *toDate);
    std::string text = integrator_.formatReport(entries);
    showTextWindow("Отчёт по водителю", text);
}

void IntegratorGUI::CallbackLoadDrivers(Fl_Widget*, void* data) {
    static_cast<IntegratorGUI*>(data)->handleLoadDrivers();
}

void IntegratorGUI::CallbackLoadOrders(Fl_Widget*, void* data) {
    static_cast<IntegratorGUI*>(data)->handleLoadOrders();
}

void IntegratorGUI::CallbackSave(Fl_Widget*, void* data) {
    static_cast<IntegratorGUI*>(data)->handleSave();
}

void IntegratorGUI::CallbackSaveStructures(Fl_Widget*, void* data) {
    static_cast<IntegratorGUI*>(data)->handleSaveStructures();
}

void IntegratorGUI::CallbackClear(Fl_Widget*, void* data) {
    static_cast<IntegratorGUI*>(data)->handleClear();
}

void IntegratorGUI::CallbackCreateDriverTable(Fl_Widget*, void* data) {
    static_cast<IntegratorGUI*>(data)->handleCreateDriverTable();
}

void IntegratorGUI::CallbackClearDriverTableOnly(Fl_Widget*, void* data) {
    static_cast<IntegratorGUI*>(data)->handleClearDriverTableOnly();
}

void IntegratorGUI::CallbackShowDriverTable(Fl_Widget*, void* data) {
    static_cast<IntegratorGUI*>(data)->handleShowDriverTable();
}

void IntegratorGUI::CallbackAddDriver(Fl_Widget*, void* data) {
    static_cast<IntegratorGUI*>(data)->handleAddDriver();
}

void IntegratorGUI::CallbackUpdateDriver(Fl_Widget*, void* data) {
    static_cast<IntegratorGUI*>(data)->handleUpdateDriver();
}

void IntegratorGUI::CallbackRemoveDriver(Fl_Widget*, void* data) {
    static_cast<IntegratorGUI*>(data)->handleRemoveDriver();
}

void IntegratorGUI::CallbackFindDriver(Fl_Widget*, void* data) {
    static_cast<IntegratorGUI*>(data)->handleFindDriver();
}

void IntegratorGUI::CallbackAddOrder(Fl_Widget*, void* data) {
    static_cast<IntegratorGUI*>(data)->handleAddOrder();
}

void IntegratorGUI::CallbackUpdateOrder(Fl_Widget*, void* data) {
    static_cast<IntegratorGUI*>(data)->handleUpdateOrder();
}

void IntegratorGUI::CallbackRemoveOrder(Fl_Widget*, void* data) {
    static_cast<IntegratorGUI*>(data)->handleRemoveOrder();
}

void IntegratorGUI::CallbackCheckOrder(Fl_Widget*, void* data) {
    static_cast<IntegratorGUI*>(data)->handleCheckOrder();
}

void IntegratorGUI::CallbackCreateOrderTree(Fl_Widget*, void* data) {
    static_cast<IntegratorGUI*>(data)->handleCreateOrderTree();
}

void IntegratorGUI::CallbackClearOrderTreeOnly(Fl_Widget*, void* data) {
    static_cast<IntegratorGUI*>(data)->handleClearOrderTreeOnly();
}

void IntegratorGUI::CallbackShowOrderTree(Fl_Widget*, void* data) {
    static_cast<IntegratorGUI*>(data)->handleShowOrderTree();
}

void IntegratorGUI::CallbackShowDateTree(Fl_Widget*, void* data) {
    static_cast<IntegratorGUI*>(data)->handleShowDateTree();
}

void IntegratorGUI::CallbackGenerateReport(Fl_Widget*, void* data) {
    static_cast<IntegratorGUI*>(data)->handleGenerateReport();
}

} // namespace

int main(int argc, char** argv) {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
    (void)argc;
    (void)argv;
    Fl::scheme("plastic");
    IntegratorGUI gui;
    gui.show();
    return Fl::run();
}

