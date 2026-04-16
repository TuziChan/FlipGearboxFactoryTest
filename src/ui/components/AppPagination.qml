pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts

Item {
    id: root

    required property AppTheme theme
    property int currentPage: 1
    property int totalPages: 1
    property int maxVisiblePages: 5
    signal pageChanged(int page)

    function setPage(page) {
        const clamped = Math.max(1, Math.min(root.totalPages, page))
        if (clamped === root.currentPage)
            return true
        root.currentPage = clamped
        root.pageChanged(root.currentPage)
        return true
    }

    function nextPage() {
        return root.setPage(root.currentPage + 1)
    }

    function previousPage() {
        return root.setPage(root.currentPage - 1)
    }

    function visiblePages() {
        const pages = []
        if (root.totalPages <= 0)
            return pages

        const maxVisible = Math.max(3, root.maxVisiblePages)
        let start = Math.max(1, root.currentPage - Math.floor(maxVisible / 2))
        let end = Math.min(root.totalPages, start + maxVisible - 1)
        start = Math.max(1, end - maxVisible + 1)

        if (start > 1)
            pages.push(1)
        if (start > 2)
            pages.push("...")

        for (let page = start; page <= end; ++page)
            pages.push(page)

        if (end < root.totalPages - 1)
            pages.push("...")
        if (end < root.totalPages)
            pages.push(root.totalPages)

        return pages
    }

    implicitWidth: pageRow.implicitWidth
    implicitHeight: 32

    RowLayout {
        id: pageRow
        anchors.fill: parent
        spacing: 6

        AppButton {
            theme: root.theme
            text: "Prev"
            size: "sm"
            variant: "ghost"
            disabled: root.currentPage <= 1
            onClicked: root.previousPage()
        }

        Repeater {
            model: root.visiblePages()

            delegate: AppButton {
                required property var modelData
                theme: root.theme
                text: String(modelData)
                size: "icon-sm"
                variant: modelData === root.currentPage ? "outline" : "ghost"
                disabled: modelData === "..."
                onClicked: root.setPage(Number(modelData))
            }
        }

        AppButton {
            theme: root.theme
            text: "Next"
            size: "sm"
            variant: "ghost"
            disabled: root.currentPage >= root.totalPages
            onClicked: root.nextPage()
        }
    }
}
