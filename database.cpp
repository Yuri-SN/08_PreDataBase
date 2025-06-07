#include "database.h"

DataBase::DataBase(QObject *parent)
    : QObject{parent}
{
    dataBase = new QSqlDatabase();
}

DataBase::~DataBase()
{
    delete dataBase;
}

/*!
 * \brief Метод добавляет БД к экземпляру класса QSqlDataBase
 * \param driver драйвер БД
 * \param nameDB имя БД (Если отсутствует Qt задает имя по умолчанию)
 */
void DataBase::AddDataBase(QString driver, QString nameDB)
{
    *dataBase = QSqlDatabase::addDatabase(driver, nameDB);
}

/*!
 * \brief Метод подключается к БД
 * \param для удобства передаем контейнер с данными необходимыми для подключения
 * \return возвращает тип ошибки
 */
void DataBase::ConnectToDataBase(QVector<QString> data)
{
    dataBase->setHostName(data[hostName]);
    dataBase->setDatabaseName(data[dbName]);
    dataBase->setUserName(data[login]);
    dataBase->setPassword(data[pass]);
    dataBase->setPort(data[port].toInt());

    dataBase->setHostName("981757-ca08998.tmweb.ru");
    dataBase->setDatabaseName("netology_cpp");
    dataBase->setUserName("netology_usr_cpp");
    dataBase->setPassword("CppNeto3");
    dataBase->setPort(5432);

    bool status = dataBase->open();

    emit sig_SendStatusConnection(status);
}

/*!
 * \brief Метод производит отключение от БД
 * \param Имя БД
 */
void DataBase::DisconnectFromDataBase(QString nameDb)
{
    *dataBase = QSqlDatabase::database(nameDb);
    dataBase->close();
}

QSqlTableModel *DataBase::GetAllFilms()
{
    QSqlTableModel *model = new QSqlTableModel(nullptr, *dataBase);
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

    model->setQuery(query, *dataBase);

    model->setHeaderData(0, Qt::Horizontal, tr("Название фильма"));
    model->setHeaderData(1, Qt::Horizontal, tr("Описание фильма"));

    return model;
}

/*!
 * @brief Метод возвращает последнюю ошибку БД
 */
QSqlError DataBase::GetLastError()
{
    return dataBase->lastError();
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
