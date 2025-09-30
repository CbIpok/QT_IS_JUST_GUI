#if defined(_WIN32) && !defined(WIN32)
#define WIN32
#endif

#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Button.H>
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
    Fl_Group*         hashToolStrip_;
    Fl_Group*         treeToolStrip_;
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

    void handleAddDriver();
    void handleUpdateDriver();
    void handleRemoveDriver();
    void handleFindDriver();

    void handleAddOrder();
    void handleUpdateOrder();
    void handleRemoveOrder();
    void handleShowOrders();
    void handleCheckOrder();

    static void CallbackLoad(Fl_Widget*, void*);
    static void CallbackSave(Fl_Widget*, void*);
    static void CallbackSaveStructures(Fl_Widget*, void*);
    static void CallbackClear(Fl_Widget*, void*);
    static void CallbackAddDriver(Fl_Widget*, void*);
    static void CallbackUpdateDriver(Fl_Widget*, void*);
    static void CallbackRemoveDriver(Fl_Widget*, void*);
    static void CallbackFindDriver(Fl_Widget*, void*);
    static void CallbackAddOrder(Fl_Widget*, void*);
    static void CallbackUpdateOrder(Fl_Widget*, void*);
    static void CallbackRemoveOrder(Fl_Widget*, void*);
    static void CallbackShowOrders(Fl_Widget*, void*);
    static void CallbackCheckOrder(Fl_Widget*, void*);
};

IntegratorGUI::IntegratorGUI()
    : integrator_(),
      hashWindow_(nullptr),
      treeWindow_(nullptr),
      hashToolStrip_(nullptr),
      treeToolStrip_(nullptr),
      hashDisplay_(nullptr),
      treeDisplay_(nullptr),
      hashBuffer_(nullptr),
      treeBuffer_(nullptr),
      hashStatusBox_(nullptr),
      treeStatusBox_(nullptr) {
    const int windowWidth = 700;
    const int windowHeight = 700;

    const int toolStripHeight = 90;
    const int toolStripPadding = 10;
    const int buttonWidth = 160;
    const int buttonHeight = 28;
    const int buttonSpacing = 8;

    hashWindow_ = new Fl_Double_Window(windowWidth, windowHeight, "Водители (хеш-таблица)");
    hashWindow_->begin();

    hashToolStrip_ = new Fl_Group(0, 0, windowWidth, toolStripHeight);
    hashToolStrip_->box(FL_THIN_UP_BOX);
    hashToolStrip_->color(fl_rgb_color(245, 245, 245));
    hashToolStrip_->begin();

    int hashButtonX = toolStripPadding;
    int hashButtonY = toolStripPadding;

    auto placeHashButton = [&](const char* label, Fl_Callback* cb) {
        if (hashButtonX + buttonWidth > windowWidth - toolStripPadding) {
            hashButtonX = toolStripPadding;
            hashButtonY += buttonHeight + buttonSpacing;
        }
        Fl_Button* button = new Fl_Button(hashButtonX, hashButtonY, buttonWidth, buttonHeight, label);
        button->callback(cb, this);
        hashButtonX += buttonWidth + buttonSpacing;
    };

    placeHashButton("Загрузить...", &IntegratorGUI::CallbackLoad);
    placeHashButton("Сохранить...", &IntegratorGUI::CallbackSave);
    placeHashButton("Сохранить структуры...", &IntegratorGUI::CallbackSaveStructures);
    placeHashButton("Очистить", &IntegratorGUI::CallbackClear);

    placeHashButton("Добавить водителя", &IntegratorGUI::CallbackAddDriver);
    placeHashButton("Изменить водителя", &IntegratorGUI::CallbackUpdateDriver);
    placeHashButton("Удалить водителя", &IntegratorGUI::CallbackRemoveDriver);
    placeHashButton("Найти водителя", &IntegratorGUI::CallbackFindDriver);

    hashToolStrip_->end();

    hashStatusBox_ = new Fl_Box(10, toolStripHeight + 5, windowWidth - 20, 30);
    hashStatusBox_->box(FL_THIN_DOWN_BOX);
    hashStatusBox_->labelfont(FL_HELVETICA_BOLD);
    hashStatusBox_->labelsize(14);
    hashStatusBox_->align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT);
    hashStatusBox_->copy_label("Готово.");

    const int hashContentTop = toolStripHeight + 40;
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

    treeToolStrip_ = new Fl_Group(0, 0, windowWidth, toolStripHeight);
    treeToolStrip_->box(FL_THIN_UP_BOX);
    treeToolStrip_->color(fl_rgb_color(245, 245, 245));
    treeToolStrip_->begin();

    int treeButtonX = toolStripPadding;
    int treeButtonY = toolStripPadding;

    auto placeTreeButton = [&](const char* label, Fl_Callback* cb) {
        if (treeButtonX + buttonWidth > windowWidth - toolStripPadding) {
            treeButtonX = toolStripPadding;
            treeButtonY += buttonHeight + buttonSpacing;
        }
        Fl_Button* button = new Fl_Button(treeButtonX, treeButtonY, buttonWidth, buttonHeight, label);
        button->callback(cb, this);
        treeButtonX += buttonWidth + buttonSpacing;
    };

    placeTreeButton("Добавить заказ", &IntegratorGUI::CallbackAddOrder);
    placeTreeButton("Изменить заказ", &IntegratorGUI::CallbackUpdateOrder);
    placeTreeButton("Удалить заказ", &IntegratorGUI::CallbackRemoveOrder);
    placeTreeButton("Заказы водителя", &IntegratorGUI::CallbackShowOrders);
    placeTreeButton("Проверить заказ", &IntegratorGUI::CallbackCheckOrder);

    treeToolStrip_->end();

    treeStatusBox_ = new Fl_Box(10, toolStripHeight + 5, windowWidth - 20, 30);
    treeStatusBox_->box(FL_THIN_DOWN_BOX);
    treeStatusBox_->labelfont(FL_HELVETICA_BOLD);
    treeStatusBox_->labelsize(14);
    treeStatusBox_->align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT);
    treeStatusBox_->copy_label("Готово.");

    const int treeContentTop = toolStripHeight + 40;
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

    std::string hashText = integrator_.hashTableAsText();
    std::string treeText = integrator_.orderTreeAsText();

    if (hashText.empty()) hashText = "<пусто>";
    if (treeText.empty()) treeText = "<пусто>";

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

    if (integrator_.loadFromFile(*path)) {
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

void IntegratorGUI::handleAddDriver() {
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
    auto record = promptOrder();
    if (!record) return;

    if (integrator_.hasOrder(*record)) {
        showInfo("Такой заказ найден в системе.");
    }
    else {
        showInfo("Такого заказа нет.");
    }
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

} // namespace

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;
    Fl::scheme("plastic");
    IntegratorGUI gui;
    gui.show();
    return Fl::run();
}

