# Reporting and Evidence

Everything in a penetration test exists to produce the report. The commands,
the captures, the shell sessions, and the screenshots all serve one purpose:
a document the client can act on. A test that finds nothing is still a test;
a report that explains nothing is worthless.

This lesson covers the final phase: turning the engagement's notes into a
clear, honest, actionable report, and handling the evidence it relies on.

## The report is the deliverable

The client pays for a report, not for activity. The test itself produces
findings, but findings without explanation are data, not intelligence.

A good report answers three questions:

```text
What did the tester find?
Why does it matter to the client?
What should the client do about it?
```

Every finding in the report must be reproducible from the evidence the report
cites. A report that asks the client to trust the tester's word is a report
that cannot be verified, and it fails its first test.

## Report structure

A professional report follows a consistent structure:

```text
executive summary
scope and methodology
findings
each finding's detail
remediation
appendices
```

The executive summary is written for people who will never read the technical
detail. The findings sections are written for the engineers who must fix the
issues. The appendices hold the raw evidence: commands, outputs, and captured
artifacts.

## Executive summary

The executive summary is the part managers read.

It states, in plain language:

```text
what was tested
the overall security posture
the number and severity of findings
the most important risks
the recommended next steps
```

An executive summary does not use jargon. It does not say "an IDOR was
identified in the orders endpoint." It says "a user could access other users'
order data by changing a number in the request." The reader should understand
the risk without knowing the technology.

## The findings table

A findings table is the spine of the report.

Each row names:

```text
a unique finding identifier
the affected asset
the severity
a short title
the status
```

A well-formed findings table lets the client prioritize remediation at a
glance.

## Finding detail

Each finding gets its own detailed section.

The detail answers:

```text
what was found
how it was found
what an attacker could do with it
what conditions are required
what the evidence shows
how to fix it
```

The attack scenario is the heart of the section. It tells the reader what the
weakness actually means in the context of their system. The evidence is the
proof the scenario is real.

## Severity and scoring

Severity tells the client how urgently each finding matters.

Severity is not assigned by mood. It follows the CVSS model, which weighs:

```text
how the vulnerability is reached
the privileges it requires
the impact on confidentiality, integrity, and availability
the likelihood of exploitation
```

The resulting score maps to a severity band:

```text
0.0-3.9  low
4.0-6.9  medium
7.0-8.9  high
9.0-10.0 critical
```

The score is a tool, not a verdict. The tester adjusts the final severity for
the client's context: a medium score on an internet-facing service may be
reported as high because the exposure makes exploitation likely.

## Evidence handling

Evidence is the raw material of the report, and it is sensitive.

The rules:

```text
evidence is protected during the engagement
evidence is delivered with the report
evidence is retained per the engagement contract
unneeded evidence is destroyed at the end
```

Captured credentials, hashes, and internal documents are the client's data.
They are handled with the same care as the client's production data, stored
on the attacker machine only as long as the engagement requires, and removed
afterward.

## Reproducibility

Every finding is reproducible when the report contains:

```text
the exact command
the exact output
the target and time
the file that holds the evidence
```

A finding that cannot be reproduced is a liability. It may be a false
positive, a misreading, or a hallucinated detail, and the client will find
out when they try to verify it. The discipline of saving every command, taught
from the first lesson, is what makes this possible.

## Remediation

A finding is not complete without its fix.

Remediation is written as:

```text
the change that removes the weakness
the change that reduces the risk
the change that detects future attempts
```

The primary fix removes the root cause. Additional recommendations add depth
in defense: monitoring, logging, and validation that catch the issue even if
the primary fix is delayed.

## Writing honestly

Honesty is the report's only real protection.

A tester reports:

```text
what was actually done
what succeeded
what failed
what was not tested
```

A failed exploit is not hidden; it is context that shows the limits of the
assessment. An untested area is stated as untested. The client builds their
risk picture from the report, and a report that overstates coverage or
severity is a report that misleads.

## Professional tone

The report is a professional document.

The tone is:

```text
clear and concrete
free of speculation
free of blame
focused on the system, not the people
```

The goal is to describe problems and solutions, not to assign fault. A
finding written as "the application fails to validate the identifier" is
professional; the same finding written as "the developer forgot to check the
ID" is not.

## The CVSS vector

The score is derived from a vector, a string of metrics.

```text
AV  attack vector: how the vulnerability is reached
AC  attack complexity: the conditions required
PR  privileges required: what access is needed first
UI  user interaction: whether a victim must act
S   scope: whether the impact crosses a boundary
C   confidentiality impact
I   integrity impact
A   availability impact
```

Each metric takes a value, and the combination produces the vector and the
score. Reading a vector explains why a finding was scored the way it was. A
finding that requires no privileges, no interaction, and is reachable over
the network scores higher than one that needs local access.

## Severity in context

The raw score is adjusted by the client's reality.

Contextual factors:

```text
is the asset internet-facing or internal?
is the asset part of a critical process?
are compensating controls already present?
is public exploit code already available?
```

The final severity in the report is the one the client should act on. It is
defensible when the report shows both the score and the context that shaped
it.

## False positives and the report

Findings that were checked and discarded still appear, in a disciplined form.

The report distinguishes:

```text
confirmed findings: verified and reproducible
non-findings: scanner results that failed verification
out of scope: observations not assessed

```

Listing discarded candidates shows the client the verification process
worked. It also prevents the client from discovering the scanner output later
and assuming the tester missed it.

## Report formats

The report exists in the formats the client will use.

```text
the full technical report
an executive summary
the findings table
raw evidence appendices
```

A common deliverable is a single document that contains all of them, with the
executive summary at the front. The technical sections assume the reader knows
the stack; the executive summary assumes nothing.

## Anatomy of a single finding

A finding section follows one shape, and the shape makes it useful.

```text
title: a short, specific name
severity: the score and its context
affected asset: host, port, path
summary: the weakness in one paragraph
attack scenario: step by step, how an attacker exploits it
evidence: the command, output, and artifact
remediation: the primary fix and hardening steps
```

The title is specific enough to be memorable: "SSH permits password-only
authentication" instead of "weak authentication." The scenario is written so
an engineer can replay it. The remediation is written so a patch can be
planned. A finding that follows this shape is ready for a remediation meeting.

## Review before delivery

The report is reviewed before it is delivered.

A final review checks:

```text
every finding has evidence
every evidence file is referenced
every severity matches the scoring
every remediation is actionable
the executive summary matches the details
the scope matches the engagement
```

The review is the last quality gate. A report that passes it is a report the
tester can stand behind.

## Next step

Now type the code version of this lesson: assemble the engagement's evidence
into a report structure, fill the findings table, and produce the final
deliverable.