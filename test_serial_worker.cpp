#include <QCoreApplication>
#include <QDebug>
#include <QTimer>
#include "src/infrastructure/bus/ModbusRtuBusController.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    qDebug() << "=== Serial Worker Thread Safety Test ===";
    qDebug() << "Main thread ID:" << QThread::currentThreadId();

    Infrastructure::Bus::ModbusRtuBusController controller;

    qDebug() << "\nAttempting to open COM23...";
    bool success = controller.open("COM23", 115200, 500, "None", 1);

    if (success) {
        qDebug() << "✓ Port opened successfully!";

        // Test sending a Modbus request
        QByteArray request;
        request.append(static_cast<char>(0x01)); // Slave ID
        request.append(static_cast<char>(0x03)); // Function code
        request.append(static_cast<char>(0x00)); // Start address high
        request.append(static_cast<char>(0x00)); // Start address low
        request.append(static_cast<char>(0x00)); // Quantity high
        request.append(static_cast<char>(0x01)); // Quantity low
        request.append(static_cast<char>(0x84)); // CRC low
        request.append(static_cast<char>(0x0A)); // CRC high

        QByteArray response;
        qDebug() << "\nSending Modbus request...";
        if (controller.sendRequest(request, response)) {
            qDebug() << "✓ Request succeeded, response size:" << response.size();
        } else {
            qDebug() << "✗ Request failed:" << controller.lastError();
        }

        controller.close();
        qDebug() << "✓ Port closed";
    } else {
        qDebug() << "✗ Failed to open port:" << controller.lastError();
    }

    qDebug() << "\n=== Test Complete ===";

    QTimer::singleShot(100, &app, &QCoreApplication::quit);
    return app.exec();
}
