#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->statusbar->addPermanentWidget(ui->connectionLabel);


    //DATABSE--------------------------------------------------------------------------------------------
    {

    IESDataBase = QSqlDatabase::addDatabase("QSQLITE");
    IESDataBase.setDatabaseName("C:/Users/PC/Documents/H.S/ProjetIES/IESDataBase.db");

    if(IESDataBase.open()){

        ui->connectionLabel->setText("Connexion à la base de données OK...!");
    } else{

       ui->connectionLabel->setText("Impossible de se connecter à la base de données");
    }

    IESDataBase.open();
    this->modelPlayer = new QSqlQueryModel();
    modelPlayer->setQuery("SELECT lname,fname,license,tag_rfid, datation_instant_shoot FROM Players");
    //modelPlayer->setQuery("SELECT * FROM Players");
    ui->playerTableView->setModel(modelPlayer);
    ui->showPlayerTable->setModel(modelPlayer);

    IESDataBase.open();
    this->modelTraining = new QSqlQueryModel();
    modelTraining->setQuery("SELECT * FROM Seance");
    ui->trainingTableView->setModel(modelTraining);




    //DATABSE--------------------------------------------------------------------------------------------
    }

    //SERIALPORT-----------------------------------------------------------------------------------------
    {

    listeDesPorts = QSerialPortInfo::availablePorts();
    for(int i = 0; i<listeDesPorts.length(); i++){
        ui->comboBoxPort->addItem(listeDesPorts[i].portName());
    }
    peripherique = new QSerialPort(this);

    connect(peripherique, SIGNAL(readyRead()), this, SLOT(lirePeripherique()));

    //SERIALPORT-----------------------------------------------------------------------------------------
    }

    //IMAGES---------------------------------------------------------------------------------------------

    QPixmap greenCircle("C:/Users/PC/Documents/H.S/ProjetIES/Ressource/voyant_vert.png");

        QPixmap redCircle("C:/Users/PC/Documents/H.S/ProjetIES/Ressource/voyant_rouge.png");

        ui->greenCLabel->setPixmap(greenCircle.scaled(120,120, Qt::KeepAspectRatioByExpanding));

        ui->redCLabel->setPixmap(redCircle.scaled(120,120, Qt::KeepAspectRatioByExpanding));

        ui->redCLabel->hide();


    //IMAGES---------------------------------------------------------------------------------------------
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::lirePeripherique()
{

    serialData = peripherique->readLine();
    serialBuffer += QString::fromStdString(serialData.toStdString());
    ui->label_readPeripherique->setText(serialBuffer);

}


void MainWindow::on_rfidEdit_returnPressed()
{

    QSqlQuery query;

    QString rfidEDIT = ui->rfidEdit->text();

    replacment(rfidEDIT);

    int counter = 0;

    if(rfidEDIT.length() == 10){

        query.exec("SELECT lname,fname,license,tag_rfid, datation_instant_shoot FROM Players WHERE tag_rfid =  \""+ rfidEDIT + "\"");
        modelPlayer->setQuery("SELECT lname,fname,license,tag_rfid, datation_instant_shoot FROM Players WHERE tag_rfid =  \""+ rfidEDIT + "\"");


        while(query.next()){ //On recherche avec une boucle dans la base de données si le TAG est attribué.

                          qDebug() << "counter" << counter;
                          counter++;
                          qDebug() << "counter" << counter;

        }

        if(counter == 0){


            QMessageBox::warning(this,"Information!","Aucun enregistrement n'a été fait avec ce tag, veuillez ajouter un joueur avec ce tag pour l'utiliser");

        }

    }



}

void MainWindow::on_delEdit_returnPressed()
{

    QString delrfid = ui->delEdit->text(); //Suppression d'un joueur avec son Tag
        replacment(delrfid);



        if(delrfid.length() != 10){

        }else{
            QMessageBox::StandardButton reponse;
            reponse = QMessageBox::question(this,"Information","Voulez-vous supprimer le joueur en question ?", QMessageBox::Yes | QMessageBox::No);

            if(reponse == QMessageBox::Yes){

                modelPlayer->setQuery("DELETE FROM Players WHERE tag_rfid = \"" +delrfid+ "\"");

                ui->delEdit->clear();

                modelPlayer->setQuery("SELECT lname,fname,tag_rfid FROM Players");

            }else{


                 }
        }

}

void MainWindow::on_addPlayerB_clicked()
{

    QSqlQuery query;
    QString addRFID = ui->addRFID->text();

    replacment(addRFID);

    QString lname = ui->lnameEdit->text();
    QString fname = ui->fnameEdit->text();


    if(addRFID.length() == 10){

        query.prepare("INSERT INTO Players (lname, fname, tag_rfid)" //Commande pour ajouter un joueur...
              "VALUES (?, ?, ?)");
        query.addBindValue(fname);                                        //...avec les saisies précédentes
        query.addBindValue(lname);
        query.addBindValue(addRFID);



        if(query.exec()){ //On vérifie si le joueur a été enregistré

            QMessageBox::information(this,"Information","Enregistrment effectué");
            modelPlayer->setQuery("SELECT lname,fname,tag_rfid FROM Players"); //On affiche le joueur.


        }else{ //Dans le cas ou il y a une erreur

            QMessageBox::information(this,"","Erreur..." + query.lastError().text());
        }


    }else{


           QMessageBox::warning(this,"Information","Le n° du tag doit comporter 10 chiffres !");


    }

}

void MainWindow::replacment(QString &rfid)
{

    //Remplacement des caractères en cas de majuscule désactivé

    rfid.replace("&","1");
    rfid.replace("é","2");
    rfid.replace("\"","3");
    rfid.replace("'","4");
    rfid.replace("(","5");
    rfid.replace("-","6");
    rfid.replace("è","7");
    rfid.replace("_","8");
    rfid.replace("ç","9");
    rfid.replace("à","0");
    rfid.replace("\r\n", NULL);
    rfid.replace("\r", NULL);
    rfid.replace("\n", NULL);

}

void MainWindow::delay(int s)
{

    QTime dieTime= QTime::currentTime().addSecs(s);
            while (QTime::currentTime() < dieTime)
    QCoreApplication::processEvents(QEventLoop::AllEvents, 100);

}

void MainWindow::on_playerTableView_doubleClicked(const QModelIndex &index)
{


    qDebug() << "index: " << index.data(2).toString() ;

    qDebug() << "ligne: " << index.row();

}

void MainWindow::on_activePortButton_clicked()
{


    if(peripherique->isOpen()){

        peripherique->close();
    }

    peripherique->setPortName(portSelectionne);
    peripherique->setBaudRate(QSerialPort::Baud9600);
    peripherique->setDataBits(QSerialPort::Data8);
    peripherique->setParity(QSerialPort::NoParity);
    peripherique->setFlowControl(QSerialPort::NoFlowControl);
    peripherique->setStopBits(QSerialPort::OneStop);

    connecte = peripherique->open(QIODevice::ReadWrite);

    if((connecte && out.contains("USB"))){

        QMessageBox::warning(this,"Information", "Laser pret !");

    }else{

        QMessageBox::warning(this,"Information", "Erreur Port Serie... !");

    }

    peripherique->clear();
}

void MainWindow::on_comboBoxPort_currentIndexChanged(const QString &arg1)
{
    int sel=ui->comboBoxPort->currentData().toInt();
        out="Siretta Modem Connection Info: \n";
        out+=listeDesPorts[sel].portName()+" "+listeDesPorts[sel].description()+"\n";
        out+=listeDesPorts[sel].systemLocation()+"\n";
        if (listeDesPorts[sel].hasVendorIdentifier()&& listeDesPorts[sel].hasProductIdentifier())
           out+=listeDesPorts[sel].manufacturer()+" ("+ QString::number(listeDesPorts[sel].vendorIdentifier(),16) + ":" + QString::number(listeDesPorts[sel].productIdentifier(),16) + ")";

        ui->label_readPeripherique->setText(out);

        portSelectionne = listeDesPorts[sel].portName();
        ui->label_Port->setText("Port : " + portSelectionne);

}

void MainWindow::on_laserON_clicked()
{
    peripherique->write("1");
}

void MainWindow::on_laserOFF_clicked()
{
    peripherique->write("0");
}
