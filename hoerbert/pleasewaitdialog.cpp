#include <QApplication>
#include <QLabel>
#include <QPushButton>
#include <QProgressBar>

#include "pleasewaitdialog.h"

PleaseWaitDialog::PleaseWaitDialog(QDialog *parent) : QDialog(parent)
{
    setWindowTitle(tr("Please wait"));
    resize(387, 146);
    m_allowClose = false;

    m_label = new QLabel(this);
    m_label->setGeometry(QRect(10, 10, 361, 101));
    m_label->setAlignment(Qt::AlignCenter);
    m_label->setText( tr("This might take several minutes.") );

    m_closeButton = new QPushButton(this);
    m_closeButton->setGeometry(QRect(270, 110, 112, 32));
    m_closeButton->setText(tr("OK"));
    m_closeButton->setDisabled(true);

    connect( m_closeButton, &QAbstractButton::clicked, this, &QWidget::close);

    QMetaObject::connectSlotsByName(this);

    m_progressBar = new QProgressBar(this);
    m_progressBar->setRange(0, 0);
    m_progressBar->setValue(0);
    m_progressBar->setGeometry(QRect(32, 110, 220, 32));
    m_progressBar->show();

    QApplication::setOverrideCursor(Qt::WaitCursor);    // hint to background action
    qApp->processEvents();
}


void PleaseWaitDialog::setResultString( QString resultString )
{
    if( resultString.isEmpty() ){
        resultString = "Finished.";
    }

    QApplication::restoreOverrideCursor();
    setCursor(Qt::ArrowCursor);
    qApp->processEvents();

    m_label->setText( resultString );
    m_closeButton->setDisabled(false);
    m_progressBar->hide();

    m_allowClose = true;
}

void PleaseWaitDialog::reject()
{
    if( m_allowClose ){
        done(0);
    }
}


