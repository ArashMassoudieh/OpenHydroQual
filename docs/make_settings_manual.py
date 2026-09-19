"""Generate the settings manual from resources/settings.json.

One subsection per field, titled with the label the user sees in the interface,
so the table of contents doubles as an index: a reader looking up one field
finds it directly. The JSON is the authoritative source -- it is what the
interface builds its dialogs from and what the solver validates against -- so
the manual is generated rather than transcribed, and cannot drift from it.
"""
import json, os, re

HERE = os.path.dirname(os.path.abspath(__file__))
SRC = os.path.join(HERE, "..", "resources", "settings.json")
OUT = os.path.join(HERE, "ohq_settings_manual.tex")

ORDER = [("general_parameters", "General settings"),
         ("solver_settings", "Solver settings"),
         ("optimizer", "Optimization"),
         ("MCMC", "MCMC")]

INTRO = {
 "General settings":
   "These fields define what period is simulated and where the results are "
   "written. Most models need nothing beyond this group.",
 "Solver settings":
   "These control how the equations are solved. The defaults are appropriate "
   "for ordinary problems; the fields below explain when a problem is not "
   "ordinary and which one to reach for.",
 "Optimization":
   "Used when the program is asked to find the parameter values that best "
   "reproduce a set of measurements. The method is a genetic algorithm: it "
   "keeps a collection of candidate parameter sets, scores each by running the "
   "whole model, and builds the next collection from those that scored best. "
   "These fields are ignored unless a calibration is run.",
 "MCMC":
   "Optimization returns the single best answer; this returns the range of "
   "answers consistent with the data. The sampler wanders through parameter "
   "space, spending time in each region in proportion to how well it explains "
   "the measurements, so the spread of what it collects measures how well each "
   "parameter is actually constrained.",
}


def esc(t):
    if t is None:
        return ""
    t = str(t)
    for a, b in (("\\", "\\textbackslash{}"), ("_", "\\_"), ("%", "\\%"),
                 ("&", "\\&"), ("#", "\\#"), ("$", "\\$"), ("~", "\\textasciitilde{}")):
        t = t.replace(a, b)
    return t


def brk(t):
    """Let long script names and file names break rather than overflow."""
    return esc(t).replace("\\_", "\\_\\allowbreak{}").replace(".", ".\\allowbreak{}")


def options(delegate):
    m = re.search(r"items:\s*(.+)$", str(delegate or ""))
    return m.group(1).strip() if m else ""


def main():
    d = json.load(open(SRC))
    L = []
    A = L.append
    A(r"\documentclass[11pt]{article}")
    A(r"\usepackage[margin=1.05in]{geometry}")
    A(r"\usepackage{booktabs}\usepackage{longtable}\usepackage{array}")
    A(r"\usepackage[colorlinks=true,linkcolor=blue!45!black,"
      r"urlcolor=blue!45!black]{hyperref}\usepackage{xcolor}")
    A(r"\usepackage{titlesec}")
    A(r"\titleformat{\subsection}{\normalfont\large\bfseries}{}{0pt}{}")
    A(r"\setlength{\parskip}{5pt}\setlength{\parindent}{0pt}")
    A(r"\title{\textbf{OpenHydroQual}\\[4pt]\large Settings Reference}")
    A(r"\author{}\date{\today}")
    A(r"\begin{document}\maketitle\thispagestyle{empty}")
    A(r"""
\section*{Using this manual}
Every field of the settings dialog has its own entry below, titled with the
label shown on screen, so the table of contents can be used as an index. Each
entry gives the default value, the available choices where a field offers them,
and the name the field carries inside a saved model file.

Fields are grouped as the interface groups them. Most models need only the
General settings; the Solver settings matter when a run misbehaves, and the
Optimization and MCMC groups only when calibrating against measurements.
\vspace{6pt}
\tableofcontents
\newpage
""")
    for key, title in ORDER:
        if key not in d:
            continue
        grp = d[key]
        A(r"\section{%s}" % esc(title))
        if title in INTRO:
            A(INTRO[title])
        items = [(k, v) for k, v in grp.items() if isinstance(v, dict) and k != "name"]

        # a compact index of the group, for scanning
        A(r"\begin{longtable}{@{}p{0.60\textwidth}p{0.32\textwidth}@{}}")
        A(r"\toprule \textrm{Field} & \textrm{Default} \\ \midrule \endhead")
        for k, v in items:
            A(r"%s & \texttt{\small %s} \\" %
              (esc(v.get("description", k)), brk(v.get("default", "")) or r"\textit{(empty)}"))
        A(r"\bottomrule\end{longtable}")

        for k, v in items:
            A(r"\subsection{%s}" % esc(v.get("description", k)))
            bits = [r"\textbf{Default:} \texttt{%s}" %
                    (brk(v.get("default", "")) or r"\textit{empty}")]
            opt = options(v.get("delegate"))
            if opt:
                bits.append(r"\textbf{Choices:} \texttt{%s}" % esc(opt))
            bits.append(r"\textbf{Name in file:} \texttt{%s}" % brk(k))
            A(r"{\small " + r" \quad\textbullet\quad ".join(bits) + r"}")
            help_ = v.get("helptext")
            if help_:
                A("")
                A(esc(help_))
    A(r"\end{document}")
    open(OUT, "w").write("\n".join(L) + "\n")
    n = sum(1 for k, _ in ORDER if k in d
            for nm, v in d[k].items() if isinstance(v, dict) and nm != "name")
    print(f"wrote {OUT} ({n} fields, one subsection each)")


if __name__ == "__main__":
    main()
