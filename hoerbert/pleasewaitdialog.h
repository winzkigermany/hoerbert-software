#ifndef PLEASEWAITDIALOG_H
#define PLEASEWAITDIALOG_H

#include <QMainWindow>
#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QProgressBar>
#include <QVBoxLayout>
#include <QCheckBox>

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
     * @brief Set the label for the checkbox
     * @param label A translated string
     */
    void setCheckBoxLabel( QString label );

    /**
     * @brief Show the "please wait" message for the long running operation
     * @param waitMessageString A translated string
     */
    void setWaitMessage( QString waitMessageString );

    /**
     * @brief is called then QDialog should be closed. We intercept that.
     */
    void reject();

    /**
     * @brief setProgressRange sets the progress bar into percentage mode with a minimum and maximum value
     * @param min
     * @param max
     */
    void setProgressRange( int min, int max );

public slots:
    /**
     * @brief setProgressValue sets the percentage value of the progress bar
     * @param percentValue
     */
    void setProgressValue( int percentValue );

    /**
     * @brief showButton show or hide the dialog's button
     * @param yesNo
     */
    void showButton( bool yesNo );

private:

    QLabel* m_label;

    QPushButton* m_closeButton;

    QProgressBar* m_progressBar;

    QVBoxLayout* m_layout;

    QCheckBox* m_checkBox;

signals:

    void checkboxIsClicked( bool );
};

#endif // PLEASEWAITDIALOG_H
