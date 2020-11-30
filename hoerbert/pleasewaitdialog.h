#ifndef PLEASEWAITDIALOG_H
#define PLEASEWAITDIALOG_H

#include <QMainWindow>
#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QProgressBar>
#include <QVBoxLayout>

class PleaseWaitDialog : public QDialog
{
    Q_OBJECT
public:

    /**
     * @brief PleaseWaitDialog constructor
     * @param parent
     */
    explicit PleaseWaitDialog(QDialog *parent = nullptr);

    /**
     * @brief Show the result of the long running operation
     * @param resultString A translated string
     */
    void setResultString( QString resultString = QString() );

    /**
     * @brief is called then QDialog should be closed. We intercept that.
     */
    void reject();

private:

    QLabel* m_label;

    QPushButton* m_closeButton;

    QProgressBar* m_progressBar;

    QVBoxLayout* m_layout;

    /**
     * @brief flag to indicate whether we inhibit closing the dialog or not.
     */
    bool m_allowClose;

signals:

};

#endif // PLEASEWAITDIALOG_H
