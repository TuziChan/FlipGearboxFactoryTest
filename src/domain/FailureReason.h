#ifndef FAILUREREASON_H
#define FAILUREREASON_H

#include <QString>

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
