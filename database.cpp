#include "database.h"

DataBase::DataBase(QObject *parent)
    : QObject{parent}
{
    m_dataBase = new QSqlDatabase();
}

DataBase::~DataBase()
{
    delete m_dataBase;
}

/*!
 * \brief Метод добавляет БД к экземпляру класса QSqlDataBase
 * \param driver драйвер БД
 * \param nameDB имя БД (Если отсутствует Qt задает имя по умолчанию)
 */
void DataBase::AddDataBase(QString driver, QString nameDB)
{
    *m_dataBase = QSqlDatabase::addDatabase(driver, nameDB);
}

/*!
 * \brief Метод подключается к БД
 * \param для удобства передаем контейнер с данными необходимыми для подключения
 * \return возвращает тип ошибки
 */
void DataBase::ConnectToDataBase(QVector<QString> data)
{
    m_dataBase->setHostName(data[hostName]);
    m_dataBase->setDatabaseName(data[dbName]);
    m_dataBase->setUserName(data[login]);
    m_dataBase->setPassword(data[pass]);
    m_dataBase->setPort(data[port].toInt());

    m_dataBase->setHostName("981757-ca08998.tmweb.ru");
    m_dataBase->setDatabaseName("netology_cpp");
    m_dataBase->setUserName("netology_usr_cpp");
    m_dataBase->setPassword("CppNeto3");
    m_dataBase->setPort(5432);

    bool status = m_dataBase->open();

    emit sig_SendStatusConnection(status);
}

/*!
 * \brief Метод производит отключение от БД
 * \param Имя БД
 */
void DataBase::DisconnectFromDataBase(QString nameDb)
{
    *m_dataBase = QSqlDatabase::database(nameDb);
    m_dataBase->close();
}

QSqlTableModel *DataBase::GetAllFilms()
{
    QSqlTableModel *model = new QSqlTableModel(nullptr, *m_dataBase);

    model->setTable("film");
    model->select();

    for (int i = 2; i < model->columnCount(); ++i) {
        model->setHeaderData(i, Qt::Horizontal, "", Qt::DisplayRole);
    }

    model->setHeaderData(0, Qt::Horizontal, "Название фильма");
    model->setHeaderData(1, Qt::Horizontal, "Описание фильма");

    return model;
}

QSqlQueryModel *DataBase::GetGenreFilms(const QString &genre)
{
    QSqlQueryModel *model = new QSqlQueryModel(this);

    QString query = QString("SELECT title, description FROM film f "
                            "JOIN film_category fc ON f.film_id = fc.film_id "
                            "JOIN category c ON c.category_id = fc.category_id "
                            "WHERE c.name = '%1'")
                        .arg(genre);

    model->setQuery(query, *m_dataBase);

    model->setHeaderData(0, Qt::Horizontal, tr("Название фильма"));
    model->setHeaderData(1, Qt::Horizontal, tr("Описание фильма"));

    return model;
}

/*!
 * @brief Метод возвращает последнюю ошибку БД
 */
QSqlError DataBase::GetLastError()
{
    return m_dataBase->lastError();
}

void DataBase::SendDataToUI(int typeRequest)
{
    QSqlTableModel *tableModel = nullptr;
    QSqlQueryModel *queryModel = nullptr;

    switch (typeRequest) {
    case requestAllFilms:
        tableModel = GetAllFilms();
        break;
    case requestComedy:
        queryModel = GetGenreFilms("Comedy");
        break;
    case requestHorrors:
        queryModel = GetGenreFilms("Horror");
        break;
    default:
        break;
    }

    emit sig_SendDataFromDB(tableModel, queryModel, typeRequest);
}
