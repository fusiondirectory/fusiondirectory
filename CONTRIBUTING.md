# Contributing to FusionDirectory projects

## Table Of Contents

1. [Welcome](#Welcome)
2. [Code of Conduct](#code-of-conduct)
3. [I just have a question!](#i-just-have-a-question)
4. [How Can I Contribute?](#how-can-i-contribute)
    * [Translate FusionDirectory in your language](#translate-fusiondirectory)
    * [Finding Wonderful Bugs](#finding-wonderful-bugs)
    * [Any Feedback To Enhance Our Apps](#any-feedback-to-enhance-our-apps)
    * [Your Awesome And Mighty Codes Contributions](#your-awesome-and-mighty-codes-contributions)
5. [Coding with style](#coding-with-style)
    * [Git Commit Messages](#git-commit-messages)
    * [PHP Style Guide](#php-style-guide)
6. [Licensing](#licensing)

---
## Welcome

> «Here's a list of our guidelines, If you want to share a best practice, or think one of these guidelines should be removed, feel free to share it with us.»

:tada: First off, thanks for taking the time to contribute!

Now, sit comfortably at the fireplace, grab your glasses and immerse yourself in our documentation.

## Code Of Conduct

All projects and everyone participating in it is governed by [FusionDirectory Code Of Conduct](./CODE_OF_CONDUCT.md). By participating, you are expected to uphold this code. Please read this before.

## I just have a question!

> **Note:** Please don't file an issue to ask a question. You'll get faster results by using the resource below.

We have an various channel of communications

*  [#fusiondirectory, the irc channel of FusionDirectory on freenode](https://webchat.freenode.net/), channel #fusiondirectory
*  [FusionDirectory Users mailing list](https://lists.fusiondirectory.org/wws/info/users)
*  [FusionDirectory dev mailing list](https://lists.fusiondirectory.org/wws/info/developpers)

## How Can I Contribute?

So, you've developed a new cool feature? Fixed an annoying bug? Found a mistake? Want to help translate FusionDirectory in your own language. We'd be happy to hear from you! :+1:

### Translate FusionDirectory

We are using [Transifex](https://www.transifex.com/fusiondirectory/FusionDirectory-1x/) to translate FusionDirectory.

* Create a transifex account
* Ask to be added to the language group you want to translate
* Start translating

### Register for a FusionDirectory project account

To create a FusionDirectory project Account you must go to

* [FusionDirectory Sign UP](https://register.fusiondirectory.org)

### Put FusionDirectory in debug mode

* Go into FusionDirectory configuration
* Activate the displaying of PHP errors, and the TRACE debug level

### Finding Wonderful Bugs

>This section guides you through submitting a bug report. Following these guidelines helps maintainers and the community understand your report, reproduce the behavior, and find related reports.

When you are creating a bug report, please include as many details as possible. Fill out the **Bugs** template to create issues, the information it asks for helps us resolve issues faster.

Explain the problem and include additional details to help maintainers reproduce the problem:

* **Use a clear and descriptive title** for the issue to identify the problem.
* **Describe the exact steps which reproduce the problem** in as many details as possible. For example, start by explaining wich brower and it's version. When listing steps, don't just say what you did, but explain how you did it. For example, if you moved the cursor to the end of an input, explain if you used the mouse, or a keyboard.
* **Describe the behavior** you observed after following the steps and point out what exactly is the problem with that behavior.
* **Explain which behavior** you expected to see instead and why.
* **Include screenshots and animated GIFs** which show you following the described steps and clearly demonstrate the problem. You can use [this tool](http://www.cockos.com/licecap/) to record GIFs on macOS and Windows, and this tool or [this tool](https://github.com/GNOME/byzanz) on Linux.
* **Include the PHP errors and trace** collected at the debug step above
* **If the problem is linked to a specific LDAP entry** attach its LDAP dump.

Fill in a report on :

* [FusionDirectory](https://gitlab.fusiondirectory.org/fusiondirectory/fd/issues) for the core program
* [FusionDirectory Plugins](https://gitlab.fusiondirectory.org/fusiondirectory/fd-plugins/issues) for plugins.

### Any Feedback To Enhance FusionDirectory

>This section guides you through submitting an enhancement suggestion, including completely new features and minor improvements to existing functionality. Following these guidelines helps maintainers and the community understand your suggestion and find related suggestions.


When you are creating a enhancement report, please include as many details as possible. Fill out the **Enhancement** template to create thoses issues, the information it asks for helps us resolve issues faster.


* [FusionDirectory](https://gitlab.fusiondirectory.org/fusiondirectory/fd/issues) for the core program
* [FusionDirectory Plugins](https://gitlab.fusiondirectory.org/fusiondirectory/fd-plugins/issues) for plugins.

* **Use a clear and descriptive title** for the issue to identify the suggestion.
* **Provide a step-by-step description of the suggested enhancement** in as many details as possible.
* **Describe the current behavior** and **explain which behavior you expected to see instead** and why.
* **Include screenshots and animated GIFs** which help you demonstrate the steps which the suggestion is related to. You can use [this tool](http://www.cockos.com/licecap/) to record GIFs on macOS and Windows, and [this tool](https://github.com/colinkeenan/silentcast) or [this tool](https://github.com/GNOME/byzanz) on Linux.
* **Explain why this enhancement would be useful** to most FusionDirectory users.

### Your awesome And Mighty Codes Contributions

>This section guides you through submitting an enhancement suggestion, including completely new features and minor improvements to existing functionality. Following these guidelines helps maintainers and the community understand your suggestion and find related suggestions.

When you are contributing a new fonctionnality or plugin, please include as many details as possible.

Fill out the **Contribution** template to create thoses issues, the information it asks for helps us resolve issues faster.

* [FusionDirectory](https://gitlab.fusiondirectory.org/fusiondirectory/fd/issues) for the core program
* [FusionDirectory Plugins](https://gitlab.fusiondirectory.org/fusiondirectory/fd-plugins/issues) for plugins.

* **Use a clear and descriptive title** for the issue to identify your contribution
* **Provide a step-by-step description of the contribution** in as many details as possible.
* **Include screenshots and animated GIFs** which help you demonstrate what this contribution is about. You can use [this tool](http://www.cockos.com/licecap/) to record GIFs on macOS and Windows, and [this tool](https://github.com/colinkeenan/silentcast) or [this tool](https://github.com/GNOME/byzanz) on Linux.
* **Explain why this contribution would be useful** to most FusionDirectory users.
* **Follow the** [Coding style](https://fusiondirectory-developer-documentation.readthedocs.io/en/1.4/fusiondirectory/coding/codingstandards.html)
* **Use our** `PHP CodeSniffer <http://pear.php.net/package/PHP_CodeSniffer>`_ rules to check code compliance [CodeSniffer](https://fusiondirectory-developer-documentation.readthedocs.io/en/1.4/fusiondirectory/coding/codingstandards.html#checking-standards)

## Coding with style

### Git Commit Messages

>Having a good guideline for creating commits and sticking to it makes working with Git and collaborating with others a lot easier.

Here are some rules of thumb :

* Use the present tense ("Add feature" not "Added feature")
* If you have a body, separate the subject from the body with a newline
* Limit the subject line to 72 characters
* Do not end the subject line with a period
* Use imperative [mood](https://en.wikipedia.org/wiki/Imperative_mood#English) in the subject line
* If the subject is not clear enough, use the body to explain what and why as opposed to how
* Consider starting the commit message with an applicable emoji, when you feel it's an important commit :
    * :tada: When first / initial commit
    * :sparkles: When introducing new features
    * :ambulance: When fixing a bug
    * :handshake: When  merging files
    * :tractor: When you changing file structure or refactoring
    * :books: When writing docs
    * :gem: When new Release
    * :lipstick: For cosmetics purposes

#### Commit Message Format
Each commit message consists of a **header**, a **body** and a **footer**.  The header has a special format that includes a **type**, a **scope** and a **subject**:

```
<emoji> <type>(<scope>): <subject>
<BLANK LINE>
<body>
<BLANK LINE>
<footer>
```

The **header** is mandatory and the **scope** of the header is optional.

Where type can be :
  * **feat**:  A new feature
  * **fix**: A bug fix
  * **core**: Code for the Core
  * **docs**: Documenting the code or writing new documentation
  * **style**: Cosmetics purpose
  * **test**: Adding missing or correcting tests

Examples:

```
:sparkles: feat(supann): Make supannRefId multivaluated

supannRefId on Structure object is too small and mono valued

close #5685
```

### PHP style Guide

To be sure to respect our guidelines, read [Coding standards](https://fusiondirectory-developer-documentation.readthedocs.io/en/1.4/fusiondirectory/coding/index.html)

## Licensing

For all new projects and most of older one we use **GPL2+ License**. You can find it [here](../LICENSE.md).

Make sure to use resources that we have the rights to use. Copyrighted images and videos may cause legal problems.

---

This document is strongly inspired by the following resources: [ovh-ux-guidelines
](https://github.com/ovh-ux/ovh-ux-guidelines), [project-guidelines](https://github.com/wearehive/project-guidelines), [Atom Guidelines](https://github.com/atom/atom/blob/master/CONTRIBUTING.md#reporting-bugs), [Angularjs guidelines](https://github.com/angular/angular.js/blob/master/CONTRIBUTING.md).

With love :heart:
