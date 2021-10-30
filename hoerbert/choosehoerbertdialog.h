#ifndef CHOOSEHOERBERTDIALOG_H
#define CHOOSEHOERBERTDIALOG_H

#include "define.h"

#include <QMainWindow>
#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QString>
#include <QDateTime>

class ChooseHoerbertDialog : public QDialog
{
    Q_OBJECT
public:

    /**
     * @brief ChooseHoerbertDialog constructor
     * @param parent
     */
    explicit ChooseHoerbertDialog(QWidget* parent = Q_NULLPTR);

signals:
    void choseHoerbertModel( int modelVersion );

private slots:
    void confirm2011Clicked();
    void confirm2021Clicked();

private:
    QGroupBox* m_buttonGroup;
    QVBoxLayout* m_layout;
    QHBoxLayout* m_buttonHLayout;
    QVBoxLayout* m_buttonVLayout;
    QLabel* m_label;
    QLabel* m_buttonsLabel;
    QPushButton* m_model2011Button;
    QPushButton* m_model2021Button;
};

#endif // CHOOSEHOERBERTDIALOG_H
