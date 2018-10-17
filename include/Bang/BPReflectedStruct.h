#ifndef BPSTRUCT_H
#define BPSTRUCT_H

#include "Bang/Array.h"
#include "Bang/BPReflectedVariable.h"
#include "Bang/BangDefines.h"
#include "Bang/IToString.h"
#include "Bang/String.h"

namespace Bang
{
class BPReflectedStruct : public IToString
{
public:
    BPReflectedStruct();
    virtual ~BPReflectedStruct() override;

    void AddVariable(const BPReflectedVariable &prop);

    const String &GetStructName() const;
    const String &GetStructVariableName() const;
    const Array<BPReflectedVariable> &GetVariables() const;

    String ToString() const override;

    bool operator==(const BPReflectedStruct &rhs) const;
    bool operator!=(const BPReflectedStruct &rhs) const;

private:
    String m_structName = "";
    String m_structVariableName = "";
    Array<BPReflectedVariable> m_variables;

    static void FromString(String::Iterator structBegin,
                           String::Iterator structEnd,
                           BPReflectedStruct *outStruct,
                           bool *success);

    void SetStructName(const String &structName);
    void SetStructVariableName(const String &structVarName);

    String GetInitializationCode() const;
    String GetGetReflectionInfoCode() const;
    String GetWriteReflectionCode() const;
    String GetReadReflectionCode() const;

    friend class BangPreprocessor;
};
}

#endif  // BPSTRUCT_H
