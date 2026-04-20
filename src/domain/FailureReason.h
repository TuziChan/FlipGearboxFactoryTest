#ifndef FAILUREREASON_H
#define FAILUREREASON_H

#include <QString>
#include <QDebug>

namespace Domain {

/**
 * @brief Failure classification for test results
 *
 * Distinguishes between communication, process, and judgment failures.
 */
enum class FailureCategory {
    None,
    Communication,  // Device offline, CRC error, timeout
    Process,        // Flow didn't reach judgment state (e.g., lock not achieved)
    Judgment        // Data valid but out of spec
};

// QDebug operator for FailureCategory
inline QDebug operator<<(QDebug debug, FailureCategory category) {
    QDebugStateSaver saver(debug);
    switch (category) {
        case FailureCategory::None: debug.nospace() << "None"; break;
        case FailureCategory::Communication: debug.nospace() << "Communication"; break;
        case FailureCategory::Process: debug.nospace() << "Process"; break;
        case FailureCategory::Judgment: debug.nospace() << "Judgment"; break;
        default: debug.nospace() << "Unknown"; break;
    }
    return debug;
}

struct FailureReason {
    FailureCategory category;
    QString description;
    
    FailureReason()
        : category(FailureCategory::None)
        , description()
    {}
    
    FailureReason(FailureCategory cat, const QString& desc)
        : category(cat)
        , description(desc)
    {}
    
    bool hasFailure() const {
        return category != FailureCategory::None;
    }
    
    QString categoryString() const {
        switch (category) {
            case FailureCategory::None: return "None";
            case FailureCategory::Communication: return "Communication";
            case FailureCategory::Process: return "Process";
            case FailureCategory::Judgment: return "Judgment";
            default: return "Unknown";
        }
    }
};

} // namespace Domain

#endif // FAILUREREASON_H
