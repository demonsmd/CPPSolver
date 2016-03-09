#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <QString>

class Exceptions
{
public:
	Exceptions(const QString& text):text(text){}
	QString getText(){return text;}

private:
	QString text;
};

inline void ensureExp(bool exp, QString message)
{
	if (!exp) throw Exceptions(message);
}

#endif // EXCEPTIONS_H
