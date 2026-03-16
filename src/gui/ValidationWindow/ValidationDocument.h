#pragma once

#include <string>
#include <vector>

class PrologixUsbGpibAdapter;

class IValidationObserver
{
public:
    virtual ~IValidationObserver() = default;
    virtual void OnValidationDocumentChanged(const std::string& changeType) = 0;
};

class ValidationDocument
{
public:
    explicit ValidationDocument(PrologixUsbGpibAdapter& adapter);
    ~ValidationDocument() = default;

    void AddObserver(IValidationObserver* observer);
    void RemoveObserver(IValidationObserver* observer);

    const std::string& GetOutputLog() const { return m_outputLog; }

    void ClearOutput();
    bool ExportOutputLog(const std::string& filePath);

    void RunAllValidations();
    void ValidateAdapterPresence();
    void ValidateAdapterStatus();
    void ValidateGpibDevicePresence();
    void ValidateDeviceStatus();
    void ValidateScpiIdn();
    void ValidateOpcQuery();
    void ValidateSweepSettingsRoundtrip();
    void ValidateCsvRoundtrip();

private:
    void NotifyObservers(const std::string& changeType);
    void AppendToLog(const std::string& text);

    bool EnsureAdapterReady();
    static bool AlmostEqual(double lhs, double rhs, double eps = 1e-6);

    PrologixUsbGpibAdapter& m_adapter;
    std::string m_outputLog;
    std::vector<IValidationObserver*> m_observers;
};
