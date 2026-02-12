#ifndef MODEL_H
#define MODEL_H

#include <QObject>
#include <QString>

namespace model {

struct ModifyInput {
    QByteArray value;
    QStringList i_files;
    QString o_path;
    bool override_output;
    bool delete_input;
};

class Worker : public QObject {
    Q_OBJECT

public:
    QString Modify(ModifyInput&& data);

signals:
    void statusUpdated(int percentage);
};

} //namespace model

#endif // MODEL_H
