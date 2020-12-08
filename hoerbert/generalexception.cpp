#include "generalexception.h"

GeneralException::GeneralException(QString const& message, bool rollbackPossible)
{
    m_message = message;
    m_rollbackPossible = rollbackPossible;
}

GeneralException::GeneralException(const GeneralException &ge)
{
    m_message = ge.m_message;
    m_rollbackPossible = ge.m_rollbackPossible;
}

GeneralException::~GeneralException()
{

}

void GeneralException::raise() const
{
    throw *this;
}

GeneralException* GeneralException::clone() const
{
    return new GeneralException(*this);
}

const QString& GeneralException::getMessage() const
{
    return m_message;
}

void GeneralException::setRollbackPossible( bool yesNo )
{
    m_rollbackPossible = yesNo;
}

bool GeneralException::isRollbackPossible() const
{
    return m_rollbackPossible;
}


