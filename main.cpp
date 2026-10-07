#include <iostream>
#include <QtWidgets>
#include <QtSql>

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow() {
        initDb();
        buildUi();
        runQuery();
    }

private:
    QSqlQueryModel *model;
    QTableView *table;
    QLabel *status;
    QLineEdit *fId, *fFirst, *fLast, *fSsn, *fMajor, *fAddr;
    QDateEdit *fBirth;
    QDoubleSpinBox *fGpa;
    QComboBox *sField, *bOp;
    QLineEdit *sText;
    QDateEdit *bDate;

    // ---------- Database ----------
    void initDb() {
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
        db.setDatabaseName("students.db");
        if (!db.open()) {
            QMessageBox::critical(this, "Error", db.lastError().text());
            return;
        }
        QSqlQuery q;
        q.exec("CREATE TABLE IF NOT EXISTS students ("
               "ID TEXT PRIMARY KEY, FirstName TEXT NOT NULL, "
               "LastName TEXT NOT NULL, SSN TEXT, Major TEXT, "
               "Birthdate TEXT, Address TEXT, GPA REAL)");
    }

    void runQuery(const QString &where = "", const QVariantList &params = {}) {
        QSqlQuery q;
        q.prepare("SELECT ID, FirstName, LastName, SSN, Major, Birthdate, "
                  "Address, GPA FROM students " + where +
                  " ORDER BY LastName, FirstName");
        for (const auto &p : params) q.addBindValue(p);
        q.exec();
        model->setQuery(std::move(q));
        while (model->canFetchMore()) model->fetchMore();
        status->setText(QString("%1 record(s) shown").arg(model->rowCount()));
    }

    // ---------- UI ----------
    void buildUi() {
        setWindowTitle("Student Database");
        resize(950, 650);

        auto *central = new QWidget;
        auto *root = new QVBoxLayout(central);
        setCentralWidget(central);

        // Form
        auto *formBox = new QGroupBox("Student Information");
        auto *g = new QGridLayout(formBox);
        fId = new QLineEdit;    fFirst = new QLineEdit; fLast = new QLineEdit;
        fSsn = new QLineEdit;   fMajor = new QLineEdit; fAddr = new QLineEdit;
        fSsn->setPlaceholderText("123-45-6789");
        fBirth = new QDateEdit(QDate(2000, 1, 1));
        fBirth->setCalendarPopup(true);
        fBirth->setDisplayFormat("yyyy-MM-dd");
        fGpa = new QDoubleSpinBox;
        fGpa->setRange(0.0, 4.0); fGpa->setSingleStep(0.1); fGpa->setDecimals(2);

        g->addWidget(new QLabel("ID:"), 0, 0);         g->addWidget(fId, 0, 1);
        g->addWidget(new QLabel("SSN:"), 0, 2);        g->addWidget(fSsn, 0, 3);
        g->addWidget(new QLabel("First Name:"), 1, 0); g->addWidget(fFirst, 1, 1);
        g->addWidget(new QLabel("Last Name:"), 1, 2);  g->addWidget(fLast, 1, 3);
        g->addWidget(new QLabel("Major:"), 2, 0);      g->addWidget(fMajor, 2, 1);
        g->addWidget(new QLabel("Birthdate:"), 2, 2);  g->addWidget(fBirth, 2, 3);
        g->addWidget(new QLabel("Address:"), 3, 0);    g->addWidget(fAddr, 3, 1);
        g->addWidget(new QLabel("GPA:"), 3, 2);        g->addWidget(fGpa, 3, 3);
        root->addWidget(formBox);

        // Buttons
        auto *btnRow = new QHBoxLayout;
        auto *bIns = new QPushButton("Insert");
        auto *bUpd = new QPushButton("Update");
        auto *bDel = new QPushButton("Delete");
        auto *bClr = new QPushButton("Clear Form");
        for (auto *b : {bIns, bUpd, bDel, bClr}) btnRow->addWidget(b);
        root->addLayout(btnRow);
        connect(bIns, &QPushButton::clicked, this, &MainWindow::insertStudent);
        connect(bUpd, &QPushButton::clicked, this, &MainWindow::updateStudent);
        connect(bDel, &QPushButton::clicked, this, &MainWindow::deleteStudent);
        connect(bClr, &QPushButton::clicked, this, &MainWindow::clearForm);

        // Search panel
        auto *searchBox = new QGroupBox("Search");
        auto *sl = new QGridLayout(searchBox);

        sField = new QComboBox;
        sField->addItem("First Name", "FirstName");
        sField->addItem("Last Name", "LastName");
        sField->addItem("ID", "ID");
        sField->addItem("Major", "Major");
        sField->addItem("Address", "Address");
        sText = new QLineEdit;
        sText->setPlaceholderText("contains...");
        auto *bSearch = new QPushButton("Search");
        sl->addWidget(new QLabel("By:"), 0, 0);  sl->addWidget(sField, 0, 1);
        sl->addWidget(sText, 0, 2);              sl->addWidget(bSearch, 0, 3);

        bOp = new QComboBox;
        bOp->addItems({"Born before", "Born on", "Born after",
                       "Born in year", "Born in month"});
        bDate = new QDateEdit(QDate(2000, 1, 1));
        bDate->setCalendarPopup(true);
        bDate->setDisplayFormat("yyyy-MM-dd");
        auto *bBirth = new QPushButton("Search Date");
        auto *bAll = new QPushButton("Show All");
        sl->addWidget(new QLabel("Birthdate:"), 1, 0); sl->addWidget(bOp, 1, 1);
        sl->addWidget(bDate, 1, 2);                    sl->addWidget(bBirth, 1, 3);
        sl->addWidget(bAll, 2, 3);
        root->addWidget(searchBox);

        connect(bSearch, &QPushButton::clicked, this, &MainWindow::searchText);
        connect(sText, &QLineEdit::returnPressed, this, &MainWindow::searchText);
        connect(bBirth, &QPushButton::clicked, this, &MainWindow::searchBirth);
        connect(bAll, &QPushButton::clicked, this, [this] { runQuery(); });

        // Table
        model = new QSqlQueryModel(this);
        table = new QTableView;
        table->setModel(model);
        table->setSelectionBehavior(QAbstractItemView::SelectRows);
        table->setSelectionMode(QAbstractItemView::SingleSelection);
        table->setEditTriggers(QAbstractItemView::NoEditTriggers);
        table->setAlternatingRowColors(true);
        table->horizontalHeader()->setStretchLastSection(true);
        root->addWidget(table, 1);
        connect(table->selectionModel(), &QItemSelectionModel::currentRowChanged,
                this, [this](const QModelIndex &cur) { loadRow(cur.row()); });

        status = new QLabel;
        statusBar()->addWidget(status);
    }

    void loadRow(int r) {
        if (r < 0) return;
        QSqlRecord rec = model->record(r);
        fId->setText(rec.value("ID").toString());
        fFirst->setText(rec.value("FirstName").toString());
        fLast->setText(rec.value("LastName").toString());
        fSsn->setText(rec.value("SSN").toString());
        fMajor->setText(rec.value("Major").toString());
        fBirth->setDate(QDate::fromString(rec.value("Birthdate").toString(), "yyyy-MM-dd"));
        fAddr->setText(rec.value("Address").toString());
        fGpa->setValue(rec.value("GPA").toDouble());
    }

    void clearForm() {
        for (auto *e : {fId, fFirst, fLast, fSsn, fMajor, fAddr}) e->clear();
        fBirth->setDate(QDate(2000, 1, 1));
        fGpa->setValue(0.0);
    }

    bool validateForm() {
        if (fId->text().trimmed().isEmpty() || fFirst->text().trimmed().isEmpty() ||
            fLast->text().trimmed().isEmpty()) {
            QMessageBox::warning(this, "Missing data", "ID, First Name and Last Name are required.");
            return false;
        }
        static QRegularExpression ssn("^\\d{3}-\\d{2}-\\d{4}$");
        if (!fSsn->text().isEmpty() && !ssn.match(fSsn->text()).hasMatch()) {
            QMessageBox::warning(this, "Invalid SSN", "SSN must look like 123-45-6789.");
            return false;
        }
        return true;
    }

    // ---------- Insert / Update / Delete ----------
    void insertStudent() {
        if (!validateForm()) return;
        QSqlQuery q;
        q.prepare("INSERT INTO students VALUES (?,?,?,?,?,?,?,?)");
        q.addBindValue(fId->text().trimmed());
        q.addBindValue(fFirst->text().trimmed());
        q.addBindValue(fLast->text().trimmed());
        q.addBindValue(fSsn->text());
        q.addBindValue(fMajor->text().trimmed());
        q.addBindValue(fBirth->date().toString("yyyy-MM-dd"));
        q.addBindValue(fAddr->text().trimmed());
        q.addBindValue(fGpa->value());
        if (!q.exec())
            QMessageBox::warning(this, "Insert failed",
                                 "Is the ID already used?\n" + q.lastError().text());
        else { runQuery(); clearForm(); }
    }

    void updateStudent() {
        if (!validateForm()) return;
        QSqlQuery q;
        q.prepare("UPDATE students SET FirstName=?, LastName=?, SSN=?, Major=?, "
                  "Birthdate=?, Address=?, GPA=? WHERE ID=?");
        q.addBindValue(fFirst->text().trimmed());
        q.addBindValue(fLast->text().trimmed());
        q.addBindValue(fSsn->text());
        q.addBindValue(fMajor->text().trimmed());
        q.addBindValue(fBirth->date().toString("yyyy-MM-dd"));
        q.addBindValue(fAddr->text().trimmed());
        q.addBindValue(fGpa->value());
        q.addBindValue(fId->text().trimmed());
        if (!q.exec() || q.numRowsAffected() == 0)
            QMessageBox::warning(this, "Update failed", "No student with that ID exists.");
        else runQuery();
    }

    void deleteStudent() {
        QString id = fId->text().trimmed();
        if (id.isEmpty()) return;
        if (QMessageBox::question(this, "Confirm", "Delete student " + id + "?")
            != QMessageBox::Yes) return;
        QSqlQuery q;
        q.prepare("DELETE FROM students WHERE ID=?");
        q.addBindValue(id);
        q.exec();
        runQuery();
        clearForm();
    }

    // ---------- Search ----------
    void searchText() {
        QString col = sField->currentData().toString();   // from fixed list, safe
        runQuery("WHERE " + col + " LIKE ?", {"%" + sText->text().trimmed() + "%"});
    }

    void searchBirth() {
        QDate d = bDate->date();
        switch (bOp->currentIndex()) {
        case 0: runQuery("WHERE Birthdate < ?", {d.toString("yyyy-MM-dd")}); break;
        case 1: runQuery("WHERE Birthdate = ?", {d.toString("yyyy-MM-dd")}); break;
        case 2: runQuery("WHERE Birthdate > ?", {d.toString("yyyy-MM-dd")}); break;
        case 3: runQuery("WHERE strftime('%Y', Birthdate) = ?",
                     {QString::number(d.year())}); break;
        case 4: runQuery("WHERE strftime('%m', Birthdate) = ?",
                     {QString("%1").arg(d.month(), 2, 10, QChar('0'))}); break;
        }
    }
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setStyle("Fusion");
    app.setStyleSheet(R"(
        QMainWindow, QWidget { background-color: #2a2347; color: #e8def8; }
        QGroupBox { border: 1px solid #7b5ea7; border-radius: 6px;
                    margin-top: 10px; font-weight: bold; }
        QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 4px;
                           color: #d98ac7; }
        QPushButton { background-color: #b05fa8; color: white; border: none;
                      border-radius: 6px; padding: 6px 14px; font-weight: bold; }
        QPushButton:hover { background-color: #d070b8; }
        QPushButton:pressed { background-color: #8a4a9c; }
        QLineEdit, QDateEdit, QDoubleSpinBox, QComboBox {
            background-color: #3a3060; color: #f0e8ff;
            border: 1px solid #7b5ea7; border-radius: 4px; padding: 3px; }
        QComboBox QAbstractItemView { background-color: #3a3060; color: #f0e8ff;
                                      selection-background-color: #b05fa8; }
        QTableView { background-color: #2f2755; alternate-background-color: #382f63;
                     gridline-color: #4a3f7a; selection-background-color: #c76bb5;
                     selection-color: white; }
        QHeaderView::section { background-color: #5a4690; color: #ffd6f0;
                               padding: 4px; border: none; font-weight: bold; }
        QStatusBar { background-color: #1f1a38; color: #d98ac7; }
    )");
    MainWindow w;
    w.show();
    return app.exec();
}

#include "main.moc"