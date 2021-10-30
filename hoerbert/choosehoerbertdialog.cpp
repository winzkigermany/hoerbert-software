#include "choosehoerbertdialog.h"

#include <QFile>
#include <QIODevice>
#include <QTextStream>
#include <QDebug>

ChooseHoerbertDialog::ChooseHoerbertDialog(QWidget* parent) : QDialog(parent)
{
    setWindowTitle(tr("Choose your hoerbert"));
    setFixedSize(600, 396);

    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(25, 20, 25, 25);

    m_label = new QLabel(this);
    m_label->setWordWrap(true);
    m_label->setText(tr("There are two versions of hörbert: The newer model does not have a mechanical On/Off switch. Memory cards for the new hörbert do not work with hörbert 2011. For which model of hörbert do you want to edit the memory card? Please decide."));
    m_label->setAlignment(Qt::AlignCenter);
    m_layout->addWidget(m_label);

    m_buttonGroup = new QGroupBox();
    m_buttonVLayout = new QVBoxLayout();
    m_buttonHLayout = new QHBoxLayout();
    m_buttonGroup->setLayout(m_buttonVLayout);

    m_buttonsLabel = new QLabel(this);
    m_buttonsLabel->setText(tr("Please click: For what kind of hörbert is your card?"));
    m_buttonsLabel->setAlignment(Qt::AlignCenter);
    m_buttonVLayout->addWidget(m_buttonsLabel);

    m_buttonVLayout->addLayout(m_buttonHLayout);
    m_layout->addWidget(m_buttonGroup);

    QSize buttonSize(200, 200);
    m_model2011Button = new QPushButton(this);  //@TODO: How on earth can an icon be added without being pixelated by Qt?
    QIcon model2011Icon;
    model2011Icon.addFile(":/images/hoerbert_with_switch_400.png", QSize(400,400));
    model2011Icon.addFile(":/images/hoerbert_with_switch_200.png", QSize(200,200));
    m_model2011Button->setIcon(model2011Icon);
    m_model2011Button->setFixedSize(buttonSize);
    m_model2011Button->setIconSize(buttonSize);
    m_model2011Button->setToolTip(tr("hoerbert model 2011 with a mechanical switch"));
    m_buttonHLayout->addWidget(m_model2011Button);
    connect( m_model2011Button, &QAbstractButton::clicked, this, &ChooseHoerbertDialog::confirm2011Clicked );

    m_model2021Button = new QPushButton(this);  //@TODO: How on earth can an icon be added without being pixelated by Qt?
    QIcon model2021Icon;
    model2021Icon.addFile(":/images/hoerbert_without_switch_400.png", QSize(400,400));
    model2021Icon.addFile(":/images/hoerbert_without_switch_400.png", QSize(200,200));
    m_model2021Button->setIcon(model2021Icon);
    m_model2021Button->setFixedSize(buttonSize);
    m_model2021Button->setIconSize(buttonSize);
    m_model2021Button->setToolTip(tr("hoerbert without a mechanical switch"));
    m_buttonHLayout->addWidget(m_model2021Button);
    connect( m_model2021Button, &QAbstractButton::clicked, this, &ChooseHoerbertDialog::confirm2021Clicked );

    QMetaObject::connectSlotsByName(this);
}

void ChooseHoerbertDialog::confirm2011Clicked()
{
    close();
    emit choseHoerbertModel( 2011 );
}

void ChooseHoerbertDialog::confirm2021Clicked()
{
    close();
    emit choseHoerbertModel( 2021 );
}
