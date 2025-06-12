#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QSqlQueryModel>
#include <QSqlTableModel>
#include <QTableView>
#include <QtConcurrent>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_ui(new Ui::MainWindow)
{
    // Исходное состояние виджетов
    m_ui->setupUi(this);

    m_ui->lb_statusConnect->setStyleSheet("color:red");
    m_ui->pb_request->setEnabled(false);

    /*
   * Выделим память под необходимые объекты. Все они наследники
   * QObject, поэтому воспользуемся иерархией.
   */

    m_dataDb = new DbData(this);
    m_dataBase = new DataBase(this);
    m_msg = new QMessageBox(this);

    // Установим размер вектора данных для подключения к БД
    dataForConnect.resize(NUM_DATA_FOR_CONNECT_TO_DB);

    /*
   * Добавим БД используя стандартный драйвер PSQL и зададим имя.
   */
    m_dataBase->AddDataBase(POSTGRE_DRIVER, DB_NAME);

    /*
   * Устанавливаем данные для подключениея к БД.
   * Поскольку метод небольшой используем лямбда-функцию.
   */
    connect(m_dataDb, &DbData::sig_sendData, this, [&](QVector<QString> receivData) {
        dataForConnect = receivData;
    });

    /*
   * Соединяем сигнал, который передает ответ от БД с методом, который
   * отображает ответ в ПИ
   */
    connect(m_dataBase, &DataBase::sig_SendDataFromDB, this, &MainWindow::ScreenDataFromDB);

    /*
   *  Сигнал для подключения к БД
   */
    connect(m_dataBase,
            &DataBase::sig_SendStatusConnection,
            this,
            &MainWindow::ReceiveStatusConnectionToDB);
}

MainWindow::~MainWindow()
{
    if (m_tableModel) {
        delete m_tableModel;
        m_tableModel = nullptr;
    }
    if (m_queryModel) {
        delete m_queryModel;
        m_queryModel = nullptr;
    }

    delete m_ui;
}

/*!
 * @brief Слот отображает форму для ввода данных подключения к БД
 */
void MainWindow::on_act_addData_triggered()
{
    // Отобразим диалоговое окно. Какой метод нужно использовать?
    m_dataDb->show();
}

/*!
 * @brief Слот выполняет подключение к БД. И отображает ошибки.
 */

void MainWindow::on_act_connect_triggered()
{
    /*
   * Обработчик кнопки у нас должен подключаться и отключаться от БД.
   * Можно привязаться к надписи лейбла статуса. Если он равен
   * "Отключено" мы осуществляем подключение, если "Подключено" то
   * отключаемся
   */

    if (m_ui->lb_statusConnect->text() == "Отключено") {
        m_ui->lb_statusConnect->setText("Подключение");
        m_ui->lb_statusConnect->setStyleSheet("color : black");

        auto conn = [&] { m_dataBase->ConnectToDataBase(dataForConnect); };
        QtConcurrent::run(conn);

    } else {
        m_dataBase->DisconnectFromDataBase(DB_NAME);
        m_ui->lb_statusConnect->setText("Отключено");
        m_ui->act_connect->setText("Подключиться");
        m_ui->lb_statusConnect->setStyleSheet("color:red");
        m_ui->pb_request->setEnabled(false);
    }
}

/*!
 * \brief Обработчик кнопки "Получить"
 */
void MainWindow::on_pb_request_clicked()
{
    QString filter = m_ui->cb_category->currentText();

    if (filter == "Все")
        m_dataBase->SendDataToUI(requestAllFilms);
    else if (filter == "Комедия")
        m_dataBase->SendDataToUI(requestComedy);
    else if (filter == "Ужасы")
        m_dataBase->SendDataToUI(requestHorrors);
}

void MainWindow::ScreenDataFromDB(QSqlTableModel *newTableModel,
                                  QSqlQueryModel *newQueryModel,
                                  int typeRequest)
{
    // Очищаем старые модели, если они были
    if (m_tableModel) {
        delete m_tableModel;
        m_tableModel = nullptr;
    }
    if (m_queryModel) {
        delete m_queryModel;
        m_queryModel = nullptr;
    }

    // Привязываем новые модели к представлению
    switch (typeRequest) {
    case requestAllFilms:
        m_tableModel = newTableModel;
        m_ui->tb_result->setModel(m_tableModel);
        break;
    case requestComedy:
    case requestHorrors:
        m_queryModel = newQueryModel;
        m_ui->tb_result->setModel(m_queryModel);
        break;
    default:
        break;
    }
}

/*!
 * \brief Метод изменяет стотояние формы в зависимости от статуса подключения к
 * БД
 * \param status
 */
void MainWindow::ReceiveStatusConnectionToDB(bool status)
{
    if (status) {
        m_ui->act_connect->setText("Отключиться");
        m_ui->lb_statusConnect->setText("Подключено к БД");
        m_ui->lb_statusConnect->setStyleSheet("color:green");
        m_ui->pb_request->setEnabled(true);
    } else {
        m_dataBase->DisconnectFromDataBase(DB_NAME);
        m_msg->setIcon(QMessageBox::Critical);
        m_msg->setText(m_dataBase->GetLastError().text());
        m_ui->lb_statusConnect->setText("Отключено");
        m_ui->lb_statusConnect->setStyleSheet("color:red");
        m_msg->exec();
    }
}

void MainWindow::on_pb_clear_clicked()
{
    m_ui->tb_result->setModel(nullptr);
}
