#ifndef BACKUPRESTOREDIALOG_H
#define BACKUPRESTOREDIALOG_H

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

class BackupRestoreDialog : public QDialog
{
    Q_OBJECT
public:

    /**
     * @brief BackupRestoreDialog constructor
     * @param parent
     */
    explicit BackupRestoreDialog(QDialog *parent = nullptr);
    void setSourcePath(const QString &sourcePath);

signals:
    void mergeClicked(const QString &, bool);
    void overwriteClicked(const QString &, bool);
    void cancelClicked();

private slots:
    void confirmMerge();
    void confirmOverwrite();

private:

    QGroupBox* m_buttonGroup;
    QVBoxLayout* m_layout;
    QHBoxLayout* m_buttonHLayout;
    QVBoxLayout* m_buttonVLayout;
    QLabel* m_label;
    QLabel* m_buttonsLabel;
    QPushButton* m_mergeButton;
    QPushButton* m_overwriteButton;
    QPushButton* m_cancelButton;

    QString m_sourcePath;

    bool parseXml(const QString &fileName);
    QString m_infoAppVersion;
    QDateTime m_infoLastWriteDate;
    QString m_infoDriveName;
    QString m_infoByWhom;
};

#endif // BACKUPRESTOREDIALOG_H
