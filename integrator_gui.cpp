#if defined(_WIN32) && !defined(WIN32)
#define WIN32
#endif

#if defined(_MSC_VER)
#    pragma execution_character_set("utf-8")
#endif

#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Native_File_Chooser.H>
#include <FL/Fl_Text_Buffer.H>
#include <FL/Fl_Text_Display.H>
#include <FL/fl_ask.H>

#include <optional>
#include <sstream>
#include <string>
#include <vector>

#include "data_integrator.hpp"

namespace {

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
    Fl_Text_Display*  hashDisplay_;
    Fl_Text_Display*  treeDisplay_;
    Fl_Text_Buffer*   hashBuffer_;
    Fl_Text_Buffer*   treeBuffer_;
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

    void handleLoad();
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
    void handleShowOrders();
    void handleCheckOrder();
    void handleCreateOrderTree();
    void handleClearOrderTreeOnly();
    void handleShowOrderTree();
    void handleGenerateReport();

    static void CallbackLoad(Fl_Widget*, void*);
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
    static void CallbackShowOrders(Fl_Widget*, void*);
    static void CallbackCheckOrder(Fl_Widget*, void*);
    static void CallbackCreateOrderTree(Fl_Widget*, void*);
    static void CallbackClearOrderTreeOnly(Fl_Widget*, void*);
    static void CallbackShowOrderTree(Fl_Widget*, void*);
    static void CallbackGenerateReport(Fl_Widget*, void*);
};

IntegratorGUI::IntegratorGUI()
    : integrator_(),
      hashWindow_(nullptr),
      treeWindow_(nullptr),
      hashMenuBar_(nullptr),
      treeMenuBar_(nullptr),
      hashDisplay_(nullptr),
      treeDisplay_(nullptr),
      hashBuffer_(nullptr),
      treeBuffer_(nullptr),
      hashStatusBox_(nullptr),
      treeStatusBox_(nullptr) {
#if defined(_WIN32)
    Fl::set_font(FL_HELVETICA, "Segoe UI");
    Fl::set_font(FL_HELVETICA_BOLD, "Segoe UI Bold");
    Fl::set_font(FL_COURIER, "Consolas");
#else
    Fl::set_font(FL_HELVETICA, "DejaVu Sans");
    Fl::set_font(FL_HELVETICA_BOLD, "DejaVu Sans Bold");
    Fl::set_font(FL_COURIER, "DejaVu Sans Mono");
#endif
    const int windowWidth = 700;
    const int windowHeight = 700;

    hashWindow_ = new Fl_Double_Window(windowWidth, windowHeight, "Водители (хеш-таблица)");
    hashWindow_->begin();

    const int menuBarHeight = 32;
    hashMenuBar_ = new Fl_Menu_Bar(0, 0, windowWidth, menuBarHeight);
    hashMenuBar_->box(FL_THIN_UP_BOX);
    hashMenuBar_->color(fl_rgb_color(245, 245, 245));
    hashMenuBar_->textsize(13);
    hashMenuBar_->add("Загр. из файла", 0, &IntegratorGUI::CallbackLoad, this);
    hashMenuBar_->add("Выгр. в файл", 0, &IntegratorGUI::CallbackSave, this);
    hashMenuBar_->add("Доб", 0, &IntegratorGUI::CallbackAddDriver, this);
    hashMenuBar_->add("Изм", 0, &IntegratorGUI::CallbackUpdateDriver, this);
    hashMenuBar_->add("Найти", 0, &IntegratorGUI::CallbackFindDriver, this);
    hashMenuBar_->add("Удалить", 0, &IntegratorGUI::CallbackRemoveDriver, this);
    hashMenuBar_->add("Отч Табл", 0, &IntegratorGUI::CallbackShowDriverTable, this);
    hashMenuBar_->add("Созд Табл", 0, &IntegratorGUI::CallbackCreateDriverTable, this);
    hashMenuBar_->add("Удал Табл", 0, &IntegratorGUI::CallbackClearDriverTableOnly, this);
    hashMenuBar_->add("Отч Структ", 0, &IntegratorGUI::CallbackSaveStructures, this);
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

    hashDisplay_ = new Fl_Text_Display(10, hashContentTop + 25, windowWidth - 20, hashContentHeight - 25);
    hashDisplay_->box(FL_DOWN_BOX);
    hashDisplay_->textfont(FL_COURIER);
    hashDisplay_->textsize(13);
    hashDisplay_->wrap_mode(Fl_Text_Display::WRAP_AT_BOUNDS, 0);

    hashWindow_->end();
    hashWindow_->resizable(hashDisplay_);

    treeWindow_ = new Fl_Double_Window(windowWidth, windowHeight, "Заказы (AVL-дерево)");
    treeWindow_->begin();

    treeMenuBar_ = new Fl_Menu_Bar(0, 0, windowWidth, menuBarHeight);
    treeMenuBar_->box(FL_THIN_UP_BOX);
    treeMenuBar_->color(fl_rgb_color(245, 245, 245));
    treeMenuBar_->textsize(13);
    treeMenuBar_->add("Доб", 0, &IntegratorGUI::CallbackAddOrder, this);
    treeMenuBar_->add("Изм", 0, &IntegratorGUI::CallbackUpdateOrder, this);
    treeMenuBar_->add("Найти", 0, &IntegratorGUI::CallbackCheckOrder, this);
    treeMenuBar_->add("Удалить", 0, &IntegratorGUI::CallbackRemoveOrder, this);
    treeMenuBar_->add("Отч Табл", 0, &IntegratorGUI::CallbackShowOrderTree, this);
    treeMenuBar_->add("Созд Табл", 0, &IntegratorGUI::CallbackCreateOrderTree, this);
    treeMenuBar_->add("Удал Табл", 0, &IntegratorGUI::CallbackClearOrderTreeOnly, this);
    treeMenuBar_->add("Заказы (Генерация отчётов)", 0, &IntegratorGUI::CallbackGenerateReport, this);
    treeMenuBar_->add("Заказы водителя", 0, &IntegratorGUI::CallbackShowOrders, this);

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

    treeDisplay_ = new Fl_Text_Display(10, treeContentTop + 25, windowWidth - 20, treeContentHeight - 25);
    treeDisplay_->box(FL_DOWN_BOX);
    treeDisplay_->textfont(FL_COURIER);
    treeDisplay_->textsize(13);
    treeDisplay_->wrap_mode(Fl_Text_Display::WRAP_AT_BOUNDS, 0);

    treeWindow_->end();
    treeWindow_->resizable(treeDisplay_);

    hashBuffer_ = new Fl_Text_Buffer();
    treeBuffer_ = new Fl_Text_Buffer();
    hashDisplay_->buffer(hashBuffer_);
    treeDisplay_->buffer(treeBuffer_);
}

IntegratorGUI::~IntegratorGUI() {
    if (hashDisplay_) {
        hashDisplay_->buffer(nullptr);
    }
    if (treeDisplay_) {
        treeDisplay_->buffer(nullptr);
    }
    delete hashBuffer_;
    delete treeBuffer_;
    delete hashWindow_;
    delete treeWindow_;
}

void IntegratorGUI::show() {
    if (hashWindow_) hashWindow_->show();
    if (treeWindow_) treeWindow_->show();
    refreshDataViews();
}

void IntegratorGUI::refreshDataViews() {
    if (!hashBuffer_ || !treeBuffer_) return;

    std::ostringstream status;
    status << "Водителей: " << integrator_.driverCount()
           << " | Заказов: " << integrator_.orderCount();
    if (hashStatusBox_) hashStatusBox_->copy_label(status.str().c_str());
    if (treeStatusBox_) treeStatusBox_->copy_label(status.str().c_str());

    std::string hashText;
    if (!integrator_.hasDriverTable()) {
        hashText = "Нет таблицы Водителей";
    }
    else {
        hashText = integrator_.hashTableAsText();
        if (hashText.empty()) hashText = "<пусто>";
    }

    std::string treeText;
    if (!integrator_.hasOrderTree()) {
        treeText = "Нет таблицы Заказов";
    }
    else {
        treeText = integrator_.orderTreeAsText();
        if (treeText.empty()) treeText = "<пусто>";
    }

    hashBuffer_->text(hashText.c_str());
    treeBuffer_->text(treeText.c_str());
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

    std::optional<int> originalLine = promptInt("Номер строки в исходном файле (-1 если ввод вручную):", initial ? initial->originalLine : -1, -1);
    if (!originalLine) return std::nullopt;

    DriverRecord record{*license, *fio, *carBrand, *originalLine};
    return record;
}

std::optional<OrderRecord> IntegratorGUI::promptOrder(const OrderRecord* initial) {
    std::optional<std::string> license = promptNonEmpty("Номер водителя (лицензия):", initial ? initial->licenseNumber : "");
    if (!license) return std::nullopt;

    std::optional<std::string> address = promptNonEmpty("Адрес заказа:", initial ? initial->address : "");
    if (!address) return std::nullopt;

    std::optional<std::string> cost = promptNonEmpty("Стоимость:", initial ? initial->cost : "");
    if (!cost) return std::nullopt;

    std::optional<std::string> date = promptNonEmpty("Дата:", initial ? initial->date : "");
    if (!date) return std::nullopt;

    OrderRecord record{*license, *address, *cost, *date};
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

void IntegratorGUI::handleLoad() {
    auto path = promptFilePath("Выбор файла конфигурации",
                               "Введите путь к файлу конфигурации:",
                               Fl_Native_File_Chooser::BROWSE_FILE,
                               false);
    if (!path) return;

    std::size_t capacity = integrator_.driverTableCapacity();
    if (capacity == 0) {
        capacity = 1;
    }
    int defaultSize = static_cast<int>(capacity);
    auto sizeOpt = promptInt("Начальный размер хеш-таблицы:", defaultSize, 1);
    if (!sizeOpt) return;

    if (integrator_.loadFromFile(*path, static_cast<std::size_t>(*sizeOpt))) {
        updateStatus("Файл успешно загружен.");
        refreshDataViews();
        showInfo("Данные загружены.");
    }
    else {
        showError("Не удалось загрузить файл.");
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
         << "Марка: " << stored->carBrand << "\n"
         << "Исходная строка: " << stored->originalLine;
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

    std::vector<OrderRecord> orders = integrator_.ordersForDriver(*license);
    if (orders.empty()) {
        showError("Для выбранного водителя нет заказов.");
        return;
    }

    std::ostringstream list;
    list << "Заказы водителя " << *license << ":\n\n";
    for (std::size_t i = 0; i < orders.size(); ++i) {
        list << (i + 1) << ") " << orders[i].address << " | " << orders[i].cost << " | " << orders[i].date << "\n";
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

    std::vector<OrderRecord> orders = integrator_.ordersForDriver(*license);
    if (orders.empty()) {
        showError("Для выбранного водителя нет заказов.");
        return;
    }

    std::ostringstream list;
    list << "Заказы водителя " << *license << ":\n\n";
    for (std::size_t i = 0; i < orders.size(); ++i) {
        list << (i + 1) << ") " << orders[i].address << " | " << orders[i].cost << " | " << orders[i].date << "\n";
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

void IntegratorGUI::handleShowOrders() {
    if (!integrator_.hasOrderTree()) {
        showError("Дерево заказов ещё не создано.");
        return;
    }
    auto license = promptNonEmpty("Номер водителя для отображения заказов:");
    if (!license) return;

    std::vector<OrderRecord> orders = integrator_.ordersForDriver(*license);
    if (orders.empty()) {
        showInfo("У данного водителя нет заказов.");
        return;
    }

    std::ostringstream list;
    list << "Заказы водителя " << *license << ":\n\n";
    for (const auto& order : orders) {
        list << "- " << order.address << " | " << order.cost << " | " << order.date << "\n";
    }
    showInfo(list.str());
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

void IntegratorGUI::handleGenerateReport() {
    if (!integrator_.hasDriverTable() || !integrator_.hasOrderTree()) {
        showError("Создайте обе структуры данных перед формированием отчёта.");
        return;
    }

    auto license = promptNonEmpty("Номер лицензии для отчёта:");
    if (!license) return;
    auto carBrand = promptNonEmpty("Марка автомобиля:");
    if (!carBrand) return;
    auto address = promptNonEmpty("Адрес заказа:");
    if (!address) return;
    auto fromDate = promptString("Начальная дата (включительно, можно оставить пустым):", "");
    if (!fromDate) return;
    auto toDate = promptString("Конечная дата (включительно, можно оставить пустым):", "");
    if (!toDate) return;

    auto entries = integrator_.generateReport(*license, *carBrand, *address, *fromDate, *toDate);
    std::string text = integrator_.formatReport(entries);
    showTextWindow("Отчёт по водителю", text);
}

void IntegratorGUI::CallbackLoad(Fl_Widget*, void* data) {
    static_cast<IntegratorGUI*>(data)->handleLoad();
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

void IntegratorGUI::CallbackShowOrders(Fl_Widget*, void* data) {
    static_cast<IntegratorGUI*>(data)->handleShowOrders();
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

void IntegratorGUI::CallbackGenerateReport(Fl_Widget*, void* data) {
    static_cast<IntegratorGUI*>(data)->handleGenerateReport();
}

} // namespace

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;
    Fl::scheme("plastic");
    IntegratorGUI gui;
    gui.show();
    return Fl::run();
}

