// Diagnostic: dump constituent quantities of a loaded model.
#include "System.h"
#include "Script.h"
#include "Quan.h"
#include "QuanSet.h"
#include "Expression.h"
#include <QCoreApplication>
#include <QFileInfo>
#include <cstdio>

static void dumpObj(Object* o) {
    std::printf("\n== %s (type %s) ==\n", o->GetName().c_str(), o->GetType().c_str());
    QuanSet* qs = o->GetVars();
    for (auto it = qs->begin(); it != qs->end(); ++it) {
        const std::string& n = it->second.GetName();
        if (n.find(':') == std::string::npos) continue;   // constituent-scoped only
        Quan& q = it->second;
        std::string kind = tostring(q.GetType());
        std::string expr;
        if (q.GetType() == Quan::_type::expression) expr = q.GetExpression()->ToString();
        std::printf("  %-32s [%s] %s\n", n.c_str(), kind.c_str(), expr.c_str());
    }
}

int main(int argc, char* argv[]) {
    QCoreApplication app(argc, argv);
    System sys;
    sys.SetDefaultTemplatePath(std::string(argv[2]) + "/");
    sys.SetWorkingFolder(QFileInfo(QString::fromStdString(argv[1])).canonicalPath().toStdString() + "/");
    sys.SetSilent(true);
    Script scr(argv[1], &sys);
    sys.CreateFromScript(scr, std::string(argv[2]) + "/settings.json");
    std::printf("constituents: %u\n", sys.ConstituentsCount());
    for (unsigned i = 0; i < sys.BlockCount(); ++i) dumpObj(sys.block(i));
    for (unsigned i = 0; i < sys.LinksCount(); ++i) dumpObj(sys.link(i));
    return 0;
}
