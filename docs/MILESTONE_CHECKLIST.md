# PSP DevLink Milestone Checklist

Use this checklist before declaring any milestone complete.

---

## Planning

* [ ] Milestone scope agreed
* [ ] Architecture reviewed
* [ ] Repository state verified

---

## Pre-Implementation Verification

Before implementation begins, verify the following:

 Desktop build succeeds
 PSP build succeeds
 Shared modules compile successfully in both targets
 No new compiler warnings introduced (or all new warnings documented)
 Existing automated verification still passes (if applicable)

Do not begin implementation until these checks have been completed successfully.

---

## Implementation

* [ ] Required functionality implemented
* [ ] No out-of-scope features added
* [ ] Repository structure verified

---

## Verification

* [ ] Project builds successfully
* [ ] Runtime verified (if applicable)
* [ ] Warnings reviewed
* [ ] Verification evidence collected
* [ ] Public interfaces reviewed for future compatibility
* [ ] Implementation matches documented architecture
* [ ] Desktop build verified
* [ ] PSP build verified
* [ ] Milestone names and documentation are consistent

---

## Documentation

* [ ] PROJECT_STATE.md updated
* [ ] HANDOFF.md updated
* [ ] Architecture documentation updated
* [ ] README.md updated (if required)
* [ ] DECISION_LOG.md updated (if required)

---

## Git

* [ ] `.gitignore` reviewed
* [ ] `git status` clean
* [ ] Changes committed
* [ ] Changes pushed

---

## Project Audit

* [ ] Quality gates satisfied
* [ ] Milestone objectives completed
* [ ] Next milestone defined

---

A milestone is complete only when every applicable item above has been completed and verified.
