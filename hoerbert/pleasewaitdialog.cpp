#include "pleasewaitdialog.h"

PleaseWaitDialog::PleaseWaitDialog(QDialog *parent) : QDialog(parent)
{
    setWindowTitle(tr("Please wait"));
    resize(387, 146);

    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(25, 15, 25, 25);

    m_label = new QLabel(this);
    m_label->setAlignment(Qt::AlignCenter);
    m_layout->addWidget(m_label);

    m_checkBox = new QCheckBox(this);
    m_checkBox->setVisible(false);
    connect( m_checkBox, &QCheckBox::clicked, this, &PleaseWaitDialog::checkboxIsClicked );
    m_layout->addWidget(m_checkBox);

    m_progressBar = new QProgressBar(this);
    m_progressBar->setRange(0, 0);
    m_progressBar->setValue(0);
    m_progressBar->setGeometry(QRect(32, 40, 323, 32));
    m_layout->addWidget(m_progressBar);
    m_progressBar->show();

    m_closeButton = new QPushButton(this);
    m_closeButton->setGeometry(QRect(250, 100, 112, 32));
    m_closeButton->setText(tr("OK"));
    m_closeButton->setDisabled(true);
    m_layout->addWidget(m_closeButton, 0, Qt::AlignRight);
    connect( m_closeButton, &QAbstractButton::clicked, this, &QWidget::close);
    QMetaObject::connectSlotsByName(this);
}

void PleaseWaitDialog::setWaitMessage( QString waitMessageString )
{
    if( waitMessageString.isEmpty() )
    {
        m_label->setText( tr("This might take several minutes.") );
    }
    else
    {
        m_label->setText(waitMessageString);
    }
    m_closeButton->setDisabled(true);
}

void PleaseWaitDialog::setResultString( QString resultString )
{
    if( resultString.isEmpty() ){
        resultString = "Finished.";
    }

    m_label->setText( resultString );
    m_closeButton->setDisabled(false);
    m_progressBar->hide();
}

void PleaseWaitDialog::setCheckBoxLabel( QString label )
{
    m_checkBox->setText(label);
    if( label.isEmpty() )
    {
        m_checkBox->setVisible(false);
    }
    else
    {
        m_checkBox->setVisible(true);
    }
}

void PleaseWaitDialog::reject()
{
    if( m_closeButton->isEnabled() ){
        done(0);
    }
}

void PleaseWaitDialog::setProgressRange( int min, int max )
{
   if( m_progressBar->value() > max )
   {
       m_progressBar->setValue( max );
   }

   if( m_progressBar->value() < min )
   {
       m_progressBar->setValue( min );
   }

   m_progressBar->setRange( min, max );
}

void PleaseWaitDialog::setValue( int percentValue )
{
    m_progressBar->setValue( qMin( qMax( percentValue, m_progressBar->minimum() ), m_progressBar->maximum() ) );
}

void PleaseWaitDialog::showButton( bool yesNo )
{
    if( yesNo )
    {
        m_closeButton->show();
    }
    else
    {
        m_closeButton->hide();
    }
}

int PleaseWaitDialog::value()
{
    return m_progressBar->value();
}
