pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC
import "../components" as Components

Item {
    id: root
    required property Components.AppTheme theme
    property var viewModel: null
    property var torqueTelemetry: viewModel ? viewModel.torqueTelemetry : null
    property string layoutMode: "wide"
    property int metricsColumns: 3
    property bool hasSidePanel: true
    property int contentGutter: 16

    QQC.ScrollView {
        id: scrollView
        anchors.fill: parent
        clip: true

        ColumnLayout {
            width: scrollView.availableWidth
            spacing: root.contentGutter

            Components.AppCard {
                Layout.fillWidth: true
                theme: root.theme

                RowLayout {
                    width: parent.width
                    spacing: 12

                    Components.AppLabel {
                        text: "DYN200 扭矩传感器"
                        fontSize: 14
                        fontWeight: 600
                        theme: root.theme
                    }

                    Item { Layout.fillWidth: true }

                    Components.AppBadge {
                        text: root.torqueTelemetry && root.torqueTelemetry.online ? "在线" : "离线"
                        variant: root.torqueTelemetry && root.torqueTelemetry.online ? "success" : "destructive"
                        theme: root.theme
                    }
                }
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: root.contentGutter
                visible: root.hasSidePanel

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: root.contentGutter

                    Components.AppCard {
                        Layout.fillWidth: true
                        theme: root.theme

                        ColumnLayout {
                            width: parent.width
                            spacing: root.contentGutter

                            Components.AppLabel {
                                text: "实时遥测"
                                fontSize: 14
                                fontWeight: 600
                                theme: root.theme
                            }

                            Components.AppSeparator {
                                Layout.fillWidth: true
                                theme: root.theme
                            }

                            Flow {
                                Layout.fillWidth: true
                                spacing: root.contentGutter

                                Components.AppCard {
                                    width: Math.max(140, (parent.width - root.contentGutter * (root.metricsColumns - 1)) / root.metricsColumns)
                                    height: 140
                                    theme: root.theme
                                    ColumnLayout {
                                        anchors.centerIn: parent
                                        spacing: 8
                                        Components.AppLabel { text: "扭矩"; fontSize: 12; color: root.theme.textSecondary; theme: root.theme; Layout.alignment: Qt.AlignHCenter }
                                        Components.AppLabel {
                                            text: root.torqueTelemetry && root.torqueTelemetry.online ? root.torqueTelemetry.torqueNm.toFixed(2) : "--"
                                            fontSize: 36; fontWeight: 700; theme: root.theme; Layout.alignment: Qt.AlignHCenter
                                        }
                                        Components.AppLabel { text: "N·m"; fontSize: 14; color: root.theme.textSecondary; theme: root.theme; Layout.alignment: Qt.AlignHCenter }
                                    }
                                }

                                Components.AppCard {
                                    width: Math.max(140, (parent.width - root.contentGutter * (root.metricsColumns - 1)) / root.metricsColumns)
                                    height: 140
                                    theme: root.theme
                                    ColumnLayout {
                                        anchors.centerIn: parent
                                        spacing: 8
                                        Components.AppLabel { text: "转速"; fontSize: 12; color: root.theme.textSecondary; theme: root.theme; Layout.alignment: Qt.AlignHCenter }
                                        Components.AppLabel {
                                            text: root.torqueTelemetry && root.torqueTelemetry.online ? root.torqueTelemetry.speedRpm.toFixed(1) : "--"
                                            fontSize: 36; fontWeight: 700; theme: root.theme; Layout.alignment: Qt.AlignHCenter
                                        }
                                        Components.AppLabel { text: "RPM"; fontSize: 14; color: root.theme.textSecondary; theme: root.theme; Layout.alignment: Qt.AlignHCenter }
                                    }
                                }

                                Components.AppCard {
                                    width: Math.max(140, (parent.width - root.contentGutter * (root.metricsColumns - 1)) / root.metricsColumns)
                                    height: 140
                                    theme: root.theme
                                    ColumnLayout {
                                        anchors.centerIn: parent
                                        spacing: 8
                                        Components.AppLabel { text: "功率"; fontSize: 12; color: root.theme.textSecondary; theme: root.theme; Layout.alignment: Qt.AlignHCenter }
                                        Components.AppLabel {
                                            text: root.torqueTelemetry && root.torqueTelemetry.online ? root.torqueTelemetry.powerW.toFixed(1) : "--"
                                            fontSize: 36; fontWeight: 700; theme: root.theme; Layout.alignment: Qt.AlignHCenter
                                        }
                                        Components.AppLabel { text: "W"; fontSize: 14; color: root.theme.textSecondary; theme: root.theme; Layout.alignment: Qt.AlignHCenter }
                                    }
                                }
                            }
                        }
                    }
                }

                ColumnLayout {
                    Layout.preferredWidth: 280
                    spacing: root.contentGutter

                    Components.AppAlert {
                        Layout.fillWidth: true
                        variant: "default"
                        title: "只读设备"
                        description: "DYN200 扭矩传感器为只读设备，无控制功能。数据通过串口通讯实时采集。"
                        theme: root.theme
                    }

                    Components.AppCard {
                        Layout.fillWidth: true
                        theme: root.theme
                        ColumnLayout {
                            width: parent.width
                            spacing: root.contentGutter
                            Components.AppLabel { text: "通讯状态"; fontSize: 12; fontWeight: 600; theme: root.theme }
                            Components.AppSeparator { Layout.fillWidth: true; theme: root.theme }
                            Components.AppLabel {
                                text: root.torqueTelemetry && root.torqueTelemetry.online ? "通讯正常，数据实时更新中" : "设备离线，无法获取遥测数据"
                                fontSize: 12; color: root.torqueTelemetry && root.torqueTelemetry.online ? root.theme.ok : root.theme.danger
                                theme: root.theme; wrapMode: Text.WordWrap; Layout.fillWidth: true
                            }
                        }
                    }

                    Components.AppCard {
                        Layout.fillWidth: true
                        theme: root.theme
                        ColumnLayout {
                            width: parent.width
                            spacing: root.contentGutter
                            Components.AppLabel { text: "最近错误"; fontSize: 12; fontWeight: 600; theme: root.theme }
                            Components.AppSeparator { Layout.fillWidth: true; theme: root.theme }
                            Components.AppLabel {
                                text: root.viewModel && root.viewModel.torqueTelemetry
                                      ? (root.viewModel.torqueTelemetry.errorCount > 0
                                         ? "最近 " + root.viewModel.torqueTelemetry.errorCount + " 次通讯错误"
                                         : "无通讯错误")
                                      : "无数据"
                                fontSize: 12
                                color: root.theme.textSecondary
                                theme: root.theme
                                wrapMode: Text.WordWrap
                                Layout.fillWidth: true
                            }
                        }
                    }
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: root.contentGutter
                visible: !root.hasSidePanel

                Components.AppCard {
                    Layout.fillWidth: true
                    theme: root.theme
                    ColumnLayout {
                        width: parent.width
                        spacing: root.contentGutter
                        Components.AppLabel { text: "实时遥测"; fontSize: 14; fontWeight: 600; theme: root.theme }
                        Components.AppSeparator { Layout.fillWidth: true; theme: root.theme }
                        Flow {
                            Layout.fillWidth: true
                            spacing: root.contentGutter

                            Components.AppCard {
                                width: Math.max(140, (parent.width - root.contentGutter * (root.metricsColumns - 1)) / root.metricsColumns)
                                height: 140
                                theme: root.theme
                                ColumnLayout {
                                    anchors.centerIn: parent; spacing: 8
                                    Components.AppLabel { text: "扭矩"; fontSize: 12; color: root.theme.textSecondary; theme: root.theme; Layout.alignment: Qt.AlignHCenter }
                                    Components.AppLabel {
                                        text: root.torqueTelemetry && root.torqueTelemetry.online ? root.torqueTelemetry.torqueNm.toFixed(2) : "--"
                                        fontSize: 36; fontWeight: 700; theme: root.theme; Layout.alignment: Qt.AlignHCenter
                                    }
                                    Components.AppLabel { text: "N·m"; fontSize: 14; color: root.theme.textSecondary; theme: root.theme; Layout.alignment: Qt.AlignHCenter }
                                }
                            }

                            Components.AppCard {
                                width: Math.max(140, (parent.width - root.contentGutter * (root.metricsColumns - 1)) / root.metricsColumns)
                                height: 140
                                theme: root.theme
                                ColumnLayout {
                                    anchors.centerIn: parent; spacing: 8
                                    Components.AppLabel { text: "转速"; fontSize: 12; color: root.theme.textSecondary; theme: root.theme; Layout.alignment: Qt.AlignHCenter }
                                    Components.AppLabel {
                                        text: root.torqueTelemetry && root.torqueTelemetry.online ? root.torqueTelemetry.speedRpm.toFixed(1) : "--"
                                        fontSize: 36; fontWeight: 700; theme: root.theme; Layout.alignment: Qt.AlignHCenter
                                    }
                                    Components.AppLabel { text: "RPM"; fontSize: 14; color: root.theme.textSecondary; theme: root.theme; Layout.alignment: Qt.AlignHCenter }
                                }
                            }

                            Components.AppCard {
                                width: Math.max(140, (parent.width - root.contentGutter * (root.metricsColumns - 1)) / root.metricsColumns)
                                height: 140
                                theme: root.theme
                                ColumnLayout {
                                    anchors.centerIn: parent; spacing: 8
                                    Components.AppLabel { text: "功率"; fontSize: 12; color: root.theme.textSecondary; theme: root.theme; Layout.alignment: Qt.AlignHCenter }
                                    Components.AppLabel {
                                        text: root.torqueTelemetry && root.torqueTelemetry.online ? root.torqueTelemetry.powerW.toFixed(1) : "--"
                                        fontSize: 36; fontWeight: 700; theme: root.theme; Layout.alignment: Qt.AlignHCenter
                                    }
                                    Components.AppLabel { text: "W"; fontSize: 14; color: root.theme.textSecondary; theme: root.theme; Layout.alignment: Qt.AlignHCenter }
                                }
                            }
                        }
                    }
                }

                Components.AppAlert {
                    Layout.fillWidth: true
                    variant: "default"
                    title: "只读设备"
                    description: "DYN200 扭矩传感器为只读设备，无控制功能"
                    theme: root.theme
                }
            }
        }
    }
}
