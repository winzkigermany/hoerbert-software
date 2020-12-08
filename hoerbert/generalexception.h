#ifndef GENERALEXCEPTION_H
#define GENERALEXCEPTION_H

#include <QException>

class GeneralException : public QException
{
public:
    GeneralException(QString const& text="", bool rollbackPossible=false);

    GeneralException(const GeneralException& ge);

    ~GeneralException() override;

    void raise() const override;
    GeneralException *clone() const override;
    const QString& getMessage() const;

    void setRollbackPossible( bool yesNo );
    bool isRollbackPossible() const;

private:
    QString m_message;
    bool m_rollbackPossible;
};

#endif // GENERALEXCEPTION_H
