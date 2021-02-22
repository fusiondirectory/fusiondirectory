## Migrate FusionDirectory from 1.0.6 to 1.0.7

  * Upgrade fusiondirectory first

Upgrade FusionDirectory core package before other ones to avoid dependencies errors:


### Upgrade of your LDAP directory

You have to update your LDAP schemas, first update the FusionDirectory core schemas. We have put a new option in fusiondirectory-insert-schema
that allow you to update easily your schema without the use of ldif files.

The following schemas needs an update :

  * core-fd-conf.schema 
  * mail-fd.schema
  * service-fd.schema
  * systems-fd-conf.schema
  * systems-fd.schema

```shell
fusiondirectory-insert-schema -m /etc/ldap/schema/fusiondirectory/core-fd-conf.schema 
```

You must do the same procedure for all other schema needing an update

## Migrate FusionDirectory from 1.0.7.0 or 1.0.7.1 to 1.0.7.2

  * Upgrade FusionDirectory first

Upgrading FusionDirectory core package before other ones to avoid dependencies errors

  * Upgrade of your LDAP directory

You have to update your LDAP schemas, first update the FusionDirectory core schemas. We have put a new option in fusiondirectory-insert-schema that allow you to update easily your schema without the use of ldif files.

```shell
fusiondirectory-insert-schema -m /etc/ldap/schema/fusiondirectory/core-fd-conf.schema 
```

## Migrate FusionDirectory from 1.0.7.2 to 1.0.7.3

  * Upgrade FusionDirectory first

Upgrading FusionDirectory core package before other ones to avoid dependencies errors:

  * Upgrade of your LDAP directory

You have to update your LDAP schemas, first update the FusionDirectory core schemas. We have put a new option in fusiondirectory-insert-schema that allow you to update easily your schema without the use of ldif files.
In this version only the schema from the argonaut plugin needs to be update if you have the systems or argonaut plugins installed

```shell
fusiondirectory-insert-schema -m /etc/ldap/schema/fusiondirectory/argonaut-fd.schema 
```

## Migrate FusionDirectory from 1.0.7.3 to 1.0.7.4

  * Upgrade FusionDirectory first

Upgrade FusionDirectory core package before other ones to avoid dependencies errors:

## Migrate FusionDirectory from 1.0.7.4 to 1.0.8

  * Upgrade FusionDirectory first

Upgrade FusionDirectory core package before other ones to avoid dependencies errors:

### Remove the deprecated plugins

The **rolemanagement** plugin has been integrated to the core so it must be removed.

The **board** plugin has been renamed so it must be replaced

### Upgrade of your LDAP directory

You have to update your LDAP schemas, first update the FusionDirectory core schemas. 

```shell
fusiondirectory-insert-schema -m /etc/ldap/schema/fusiondirectory/core-fd.schema 

fusiondirectory-insert-schema -m /etc/ldap/schema/fusiondirectory/core-fd-conf.schema 
```

After that you have to update the schema for the plugins you are using. In this version the following schema have to be updated

  * argonaut-fd-schema
  * ipmi-fd.schema
  * repository-fd.schema

## Migrate FusionDirectory from 1.0.8 to 1.0.8.1

  * Upgrade FusionDirectory first

Upgrade FusionDirectory core package before other ones to avoid dependencies errors:

### Upgrade of LDAP directory
 
You have to update your LDAP schemas, first update the FusionDirectory core schemas. 

```shell
fusiondirectory-insert-schema -m /etc/ldap/schema/fusiondirectory/core-fd.schema 

fusiondirectory-insert-schema -m /etc/ldap/schema/fusiondirectory/core-fd-conf.schema 
```

### Check for deprecated attributes and objectClasses in your LDAP

FusionDirectory 1.0.8.1 comes with two new options in fusiondirectory-setup

fusiondirectory-setup --list-deprecated show deprecated attributes and objectClasses for FusionDirectory

```shell
fusiondirectory-setup --list-deprecated

List deprecated attributes and objectclasses
Deprecated attributes:
 gosaUnitTag                    (Takes a list of relevant mime-type|priority settings)          - 1.3.6.1.4.1.10098.1.1.12.33
 gosaSnapshotType               (Takes either undo or snapshot)                                 - 1.3.6.1.4.1.10098.1.1.12.36
 fdHonourUnitTags               (FusionDirectory - Honour unit tags)                            - 1.3.6.1.4.1.38414.8.14.3
Deprecated objectClasses:
 gosaAdministrativeUnitTag      (Marker for objects below administrational units (v2.6.1))      - 1.3.6.1.4.1.10098.1.2.1.19.16
 gosaAdministrativeUnit         (Marker for administrational units (v2.6.1))                    - 1.3.6.1.4.1.10098.1.2.1.19.15
```

fusiondirectory-setup --check-deprecated will output an ldif file on the console that you can use with ldapmodify to clean you ldap server from old attributes and objectClass

```shell
fusiondirectory-setup --check-deprecated

List LDAP entries using deprecated attributes or objectclasses
There are no entries in the LDAP using obsolete attributes
There are no entries in the LDAP using obsolete classes
```

## Migrate FusionDirectory from 1.0.8.1 to 1.0.8.2

  * Upgrade FusionDirectory first

Upgrade FusionDirectory core package before other ones to avoid dependencies errors

## Migrate FusionDirectory from 1.0.8.2 to 1.0.8.3

  * Upgrade FusionDirectory first

Upgrade FusionDirectory core package before other ones to avoid dependencies errors

### Upgrade of your LDAP directory
 
You have to update your LDAP schemas, first update the FusionDirectory core schemas. 

```shell
fusiondirectory-insert-schema -m /etc/ldap/schema/fusiondirectory/core-fd.schema 

fusiondirectory-insert-schema -m /etc/ldap/schema/fusiondirectory/core-fd-conf.schema 
```

### Check for deprecated attributes and objectClasses in your LDAP

fusiondirectory-setup --list-deprecated show deprecated attributes and objectClasses for FusionDirectory

```shell
fusiondirectory-setup --list-deprecated

List deprecated attributes and objectclasses
Deprecated attributes:
 gosaUnitTag                    (Takes a list of relevant mime-type|priority settings)          - 1.3.6.1.4.1.10098.1.1.12.33
 gosaSnapshotType               (Takes either undo or snapshot)                                 - 1.3.6.1.4.1.10098.1.1.12.36
 fdHonourUnitTags               (FusionDirectory - Honour unit tags)                            - 1.3.6.1.4.1.38414.8.14.3
Deprecated objectClasses:
 gosaAdministrativeUnitTag      (Marker for objects below administrational units (v2.6.1))      - 1.3.6.1.4.1.10098.1.2.1.19.16
 gosaAdministrativeUnit         (Marker for administrational units (v2.6.1))                    - 1.3.6.1.4.1.10098.1.2.1.19.15
```

fusiondirectory-setup --check-deprecated will output an ldif file on the console that you can use with ldapmodify to clean you ldap server from old attributes and objectClass

```shell
fusiondirectory-setup --check-deprecated

List LDAP entries using deprecated attributes or objectclasses
There are no entries in the LDAP using obsolete attributes
There are no entries in the LDAP using obsolete classes
```

!! Please read it carefully before applying !!

## Migrate FusionDirectory from 1.0.8.3 to 1.0.8.4

  * Upgrade FusionDirectory first

Upgrade FusionDirectory core package before other ones to avoid dependencies errors:

### Check for deprecated attributes and objectClasses in your LDAP

fusiondirectory-setup --list-deprecated show deprecated attributes and objectClasses for FusionDirectory

```shell
fusiondirectory-setup --list-deprecated

List deprecated attributes and objectclasses
Deprecated attributes:
 gosaUnitTag                    (Takes a list of relevant mime-type|priority settings)          - 1.3.6.1.4.1.10098.1.1.12.33
 gosaSnapshotType               (Takes either undo or snapshot)                                 - 1.3.6.1.4.1.10098.1.1.12.36
 fdHonourUnitTags               (FusionDirectory - Honour unit tags)                            - 1.3.6.1.4.1.38414.8.14.3
Deprecated objectClasses:
 gosaAdministrativeUnitTag      (Marker for objects below administrational units (v2.6.1))      - 1.3.6.1.4.1.10098.1.2.1.19.16
 gosaAdministrativeUnit         (Marker for administrational units (v2.6.1))                    - 1.3.6.1.4.1.10098.1.2.1.19.15
```

fusiondirectory-setup --check-deprecated will output an ldif file on the console that you can use with ldapmodify to clean you ldap server from old attributes and objectClass

```shell
fusiondirectory-setup --check-deprecated

List LDAP entries using deprecated attributes or objectclasses
There are no entries in the LDAP using obsolete attributes
There are no entries in the LDAP using obsolete classes
```

!! Please read it carefully before applying !!

## Migrate FusionDirectory from 1.0.8.4 to 1.0.8.5

### Remove the deprecated plugins

The **dashboard** plugin has been integrated to the core so it must be removed.

```shell
apt-get --purge remove fusiondirectory-plugin-dashboard
```

  * Upgrade FusionDirectory first

Upgrade FusionDirectory core package before other ones to avoid dependencies errors:

```shell
apt-get install fusiondirectory
```

  * Upgrade FusionDirectory schema package too.

```shell
apt-get install fusiondirectory-schema
```

### Upgrade of your LDAP directory

You may have to update your LDAP schemas, first update the FusionDirectory core schemas. 

!!! Only update this one if you don't have the dashboard.schema installed, because now that dashboard is merged to core, its attribute are in core. !!!

```shell
fusiondirectory-insert-schema -m /etc/ldap/schema/fusiondirectory/core-fd.schema 
```

  * If you are using the debconf plugin update his schema

```shell
fusiondirectory-insert-schema -m /etc/ldap/schema/fusiondirectory/debconf.schema 
```

  * If you are using the fai plugin update his schema

```shell
fusiondirectory-insert-schema -m /etc/ldap/schema/fusiondirectory/fai.schema 
```

  * If you are using the mail plugin update his schema

```shell
fusiondirectory-insert-schema -m /etc/ldap/schema/fusiondirectory/mail-fd.schema 
```

### Check for deprecated attributes and objectClasses in your LDAP

fusiondirectory-setup --list-deprecated show deprecated attributes and objectClasses for FusionDirectory

```shell
fusiondirectory-setup --list-deprecated

List deprecated attributes and objectclasses
Deprecated attributes:
 gosaUnitTag                    (Takes a list of relevant mime-type|priority settings)          - 1.3.6.1.4.1.10098.1.1.12.33
 gosaSnapshotType               (Takes either undo or snapshot)                                 - 1.3.6.1.4.1.10098.1.1.12.36
 fdHonourUnitTags               (FusionDirectory - Honour unit tags)                            - 1.3.6.1.4.1.38414.8.14.3
Deprecated objectClasses:
 gosaAdministrativeUnitTag      (Marker for objects below administrational units (v2.6.1))      - 1.3.6.1.4.1.10098.1.2.1.19.16
 gosaAdministrativeUnit         (Marker for administrational units (v2.6.1))                    - 1.3.6.1.4.1.10098.1.2.1.19.15
```

fusiondirectory-setup --check-deprecated will output an ldif file on the console that you can use with ldapmodify to clean you ldap server from old attributes and objectClass

```shell
fusiondirectory-setup --check-deprecated

List LDAP entries using deprecated attributes or objectclasses
There are no entries in the LDAP using obsolete attributes
There are no entries in the LDAP using obsolete classes
```

!! Please read it carefully before applying !!

## Migrate FusionDirectory from 1.0.8.5 to 1.0.8.6

  * Upgrade FusionDirectory first

Upgrade FusionDirectory core package before other ones to avoid dependencies errors:

```shell
apt-get install fusiondirectory
```

  * Upgrade FusionDirectory schema package too.

```shell
apt-get install fusiondirectory-schema
```

## Migrate FusionDirectory from 1.0.8.6 to 1.0.8.7

  * Upgrade FusionDirectory first

Upgrade FusionDirectory core package before other ones to avoid dependencies errors:

```shell
apt-get install fusiondirectory
```

  * Upgrade FusionDirectory schema package too.

```shell
apt-get install fusiondirectory-schema
```

### Upgrade of your LDAP directory

  * You may have to update your LDAP schemas, first update the FusionDirectory core schemas.
 
!! You need to remove the dashboard.schema if installed, because now that dashboard is merged to core, its attribute are in core. !!

```shell
fusiondirectory-insert-schema -m /etc/ldap/schema/fusiondirectory/core-fd-conf.schema 
```

### Check for deprecated attributes and objectClasses in your LDAP

fusiondirectory-setup --list-deprecated show deprecated attributes and objectClasses for FusionDirectory

```shell
fusiondirectory-setup --list-deprecated

List deprecated attributes and objectclasses
Deprecated attributes:
 gosaUnitTag                    (Takes a list of relevant mime-type|priority settings)          - 1.3.6.1.4.1.10098.1.1.12.33
 gosaSnapshotType               (Takes either undo or snapshot)                                 - 1.3.6.1.4.1.10098.1.1.12.36
 fdHonourUnitTags               (FusionDirectory - Honour unit tags)                            - 1.3.6.1.4.1.38414.8.14.3
Deprecated objectClasses:
 gosaAdministrativeUnitTag      (Marker for objects below administrational units (v2.6.1))      - 1.3.6.1.4.1.10098.1.2.1.19.16
 gosaAdministrativeUnit         (Marker for administrational units (v2.6.1))                    - 1.3.6.1.4.1.10098.1.2.1.19.15
```

fusiondirectory-setup --check-deprecated will output an ldif file on the console that you can use with ldapmodify to clean you ldap server from old attributes and objectClass

```shell
fusiondirectory-setup --check-deprecated

List LDAP entries using deprecated attributes or objectclasses
There are no entries in the LDAP using obsolete attributes
There are no entries in the LDAP using obsolete classes
```

!! Please read it carefully before applying !!

## Migrate FusionDirectory from 1.0.8.7 to 1.0.8.8

  * Upgrade FusionDirectory first

Upgrade FusionDirectory core package before other ones to avoid dependencies errors:

```shell
apt-get install fusiondirectory
```

  * Upgrade FusionDirectory schema package too.

```shell
apt-get install fusiondirectory-schema
```

### Upgrade of LDAP directory

  * Nothing to do

### Check for deprecated attributes and objectClasses in your LDAP

fusiondirectory-setup --list-deprecated show deprecated attributes and objectClasses for FusionDirectory

```shell
fusiondirectory-setup --list-deprecated

List deprecated attributes and objectclasses
Deprecated attributes:
 gosaUnitTag                    (Takes a list of relevant mime-type|priority settings)          - 1.3.6.1.4.1.10098.1.1.12.33
 gosaSnapshotType               (Takes either undo or snapshot)                                 - 1.3.6.1.4.1.10098.1.1.12.36
 fdHonourUnitTags               (FusionDirectory - Honour unit tags)                            - 1.3.6.1.4.1.38414.8.14.3
Deprecated objectClasses:
 gosaAdministrativeUnitTag      (Marker for objects below administrational units (v2.6.1))      - 1.3.6.1.4.1.10098.1.2.1.19.16
 gosaAdministrativeUnit         (Marker for administrational units (v2.6.1))                    - 1.3.6.1.4.1.10098.1.2.1.19.15
```

fusiondirectory-setup --check-deprecated will output an ldif file on the console that you can use with ldapmodify to clean you ldap server from old attributes and objectClass

```shell
fusiondirectory-setup --check-deprecated

List LDAP entries using deprecated attributes or objectclasses
There are no entries in the LDAP using obsolete attributes
There are no entries in the LDAP using obsolete classes
```

!! Please read it carefully before applying !!

## Migrate FusionDirectory from 1.0.8.8 to 1.0.8.9

  * Upgrade FusionDirectory first

Upgrade FusionDirectory core package before other ones to avoid dependencies errors

```shell
apt-get install fusiondirectory
```

  * Upgrade FusionDirectory schema package too.

```shell
apt-get install fusiondirectory-schema
```

### Upgrade of LDAP directory

  * Upgrade the argonaut.schema if you use the argonaut plugin

```shell
fusiondirectory-insert-schema -m /etc/ldap/schema/fusiondirectory/argonaut-fd.schema 
```

### Check for deprecated attributes and objectClasses in your LDAP

fusiondirectory-setup --list-deprecated show deprecated attributes and objectClasses for FusionDirectory

```shell
fusiondirectory-setup --list-deprecated

List deprecated attributes and objectclasses
Deprecated attributes:
 gosaUnitTag                    (Takes a list of relevant mime-type|priority settings)          - 1.3.6.1.4.1.10098.1.1.12.33
 gosaSnapshotType               (Takes either undo or snapshot)                                 - 1.3.6.1.4.1.10098.1.1.12.36
 fdHonourUnitTags               (FusionDirectory - Honour unit tags)                            - 1.3.6.1.4.1.38414.8.14.3
Deprecated objectClasses:
 gosaAdministrativeUnitTag      (Marker for objects below administrational units (v2.6.1))      - 1.3.6.1.4.1.10098.1.2.1.19.16
 gosaAdministrativeUnit         (Marker for administrational units (v2.6.1))                    - 1.3.6.1.4.1.10098.1.2.1.19.15
```

fusiondirectory-setup --check-deprecated will output an ldif file on the console that you can use with ldapmodify to clean you ldap server from old attributes and objectClass

```shell
fusiondirectory-setup --check-deprecated

List LDAP entries using deprecated attributes or objectclasses
There are no entries in the LDAP using obsolete attributes
There are no entries in the LDAP using obsolete classes
```

!! Please read it carefully before applying !!

## Migrate FusionDirectory from 1.0.8.9 to 1.0.9

### Backup you template First

The template system has been completely rewritten, and the old template will not be converted, so you need to backup them, remove them from FusionDirectory and then redo them.

The new system is much cleaner because template are stored in they own branch now and are the basis for having template for the whole application in the future.

### Backup your phones

You must backup all your phones in a LDIF and remove them.

Modify your LDIF like below:

  * Before

```ldif
cn=namePhone,ou=phones,ou=systems,dc=labo,dc=opensides,dc=be
cn: namePhone
goFonType: friend
goFonDmtfMode: inband
goFonDefaultIP: dynamic
ipHostNumber: 127.0.0.1
macAddress: 00:0C:7F:31:33:F1
objectClass: top
```

  * After

```ldif
cn=namePhone,ou=phones,ou=systems
cn: namePhone
ipHostNumber: 127.0.0.1
macAddress: 00:0C:7F:31:33:F1
objectClass: fdPhone
objectClass: device
objectClass: ieee802Device
objectClass: ipHost
```

After the migration you may insert your phone again in your LDAP.

### Moved Attributes

We cleaned the core of FusionDirectory to only used classical attributes and objectClass, so the FusionDirectory special attributes on the first page of FusionDirectory
are now managed by the personal plugin.

The personal plugin has been made to manage all kind of personal data and is best suited to contain those data not normalized.
So of you use personalTitle, dateOfBirth, gender you will need to install the personal plugin.

  * Upgrade FusionDirectory first

Upgrade FusionDirectory core package before other ones to avoid dependencies errors

```shell
apt-get install fusiondirectory
```

  * Upgrade FusionDirectory schema package too.

```shell
apt-get install fusiondirectory-schema
```

### Removed obsolete plugins

In this version we removed obsolete plugin, because they where based on software no longer existing
or needed an sql backend and we decided they need to be rewritten using the now ldap backend of those softwares :

  * asterisk : old plugin not maintainable anymore based on the sql backend, but now asterisk has an ldap backend has well.

  * fax : old code based on the gofax software from gonicus, not maintained anymore upstream.

  * openstack-compute : openstack management interface changed and this plugin is no longer relevant

  * uw-imap : long gone imap server, nobody that whe know still use it

They are no longer supported and should be removed from your installation

```shell
apt-get remove fusiondirectory-plugin-asterisk 

apt-get remove fusiondirectory-plugin-fax

apt-get remove fusiondirectory-plugin-openstack-compute

apt-get remove fusiondirectory-plugin-uwimap
```

### New Plugin for peoples using a mix of groupOfNames and posixGroup on the same object

We cleaned the code of FusionDirectory and by default it make posixGroup or clean standard compliant groupOfNames.
If you need a mix of groupOfNames and posixGroup on the same object you will need to install a new plugin called mixedgroups. 

! Please take note that this plugin is for special cases and need modified core schema. !

If you need to have a link between user and groups you are better using the standard compliant groupOfNames or roles for examples for web applications.

```shell
apt-get install fusiondirectory-plugin-mixedgroups 
```

### Upgrade of your LDAP directory

  * Upgrade Core FusionDirectory Schema

The password recovery schema has been migrated into the core schema so you must remove it first

```shell
fusiondirectory-insert-schema -e recovery-fd
Are you sure you want to empty schema(s) recovery-fd? [Yes/No]?
```

  * Upgrade the core-fd.schema and core-fd-conf.schema

```shell
fusiondirectory-insert-schema -m /etc/ldap/schema/fusiondirectory/core-fd.schema 

fusiondirectory-insert-schema -m /etc/ldap/schema/fusiondirectory/core-fd-conf.schema 
```

  * Upgrade the argonaut.schema if you use the argonaut plugin

```shell
fusiondirectory-insert-schema -m /etc/ldap/schema/fusiondirectory/argonaut-fd.schema 
```

  * Upgrade the fai-fd-conf.schema if you use the fai plugin

```shell
fusiondirectory-insert-schema -m /etc/ldap/schema/fusiondirectory/fai-fd-conf.schema
```

  * Upgrade the mail-fd.schema if you use the mail plugin

```shell
fusiondirectory-insert-schema -m /etc/ldap/schema/fusiondirectory/mail-fd.schema 
```

  * Upgrade the personal-fd.schema if you use the personal plugin

```shell
fusiondirectory-insert-schema -m /etc/ldap/schema/fusiondirectory/personal-fd.schema  
```

  * Upgrade the proxy-fd.schema if you use the squid plugin

```shell
fusiondirectory-insert-schema -m /etc/ldap/schema/fusiondirectory/proxy-fd.schema
```

- Upgrade the service-fd.schema, systems-fd.schema, systems-fd-conf.schema if you use the system plugin

The Phone object has been migrated to the system plugin due to the removal of the asterisk plugin.

```shell
fusiondirectory-insert-schema -e asterisk-fd-conf
Are you sure you want to empty schema(s) asterisk-fd-conf? [Yes/No]?

fusiondirectory-insert-schema -m /etc/ldap/schema/fusiondirectory/service-fd.schema

fusiondirectory-insert-schema -m /etc/ldap/schema/fusiondirectory/systems-fd-conf.schema 

fusiondirectory-insert-schema -m /etc/ldap/schema/fusiondirectory/systems-fd.schema 
```

### Check for deprecated attributes and objectClasses in your LDAP

```shell
fusiondirectory-setup --list-deprecated show deprecated attributes and objectClasses for FusionDirectory

List deprecated attributes and objectclasses
Deprecated attributes:
 gotoXResolution                (GOto - Gonicus Terminal Concept, value xResolution.)           - 1.3.6.1.4.1.10098.1.1.1.20
 ghSoundAdapter                 (Hardware definitions, value soundAdapter)                      - 1.3.6.1.4.1.10098.1.1.2.7
 gotoShare                      (GOto - specifies a share)                                      - 1.3.6.1.4.1.10098.1.1.11.9
 fdSnapshotAdminDn              (FusionDirectory - Snaphost admin dn)                           - 1.3.6.1.4.1.38414.8.17.4
 gotoXDriver                    (GOto - Gonicus Terminal Concept, value xDriver.)               - 1.3.6.1.4.1.10098.1.1.1.28
 gotoXMouseType                 (Hardware definitions, value Type of mouse)                     - 1.3.6.1.4.1.10098.1.1.1.34
 gotoLpdEnable                  (GOto - Gonicus Terminal Concept, value lpdEnable.)             - 1.3.6.1.4.1.10098.1.1.1.9
 fdAccountRDN                   (FusionDirectory - use a placeholder pattern for generating account RDNs)       - 1.3.6.1.4.1.38414.8.12.2
 gotoScannerBackend             (GOto - Gonicus Terminal Concept, value scannerBackend.)        - 1.3.6.1.4.1.10098.1.1.1.39
 gotoXVsync                     (GOto - Gonicus Terminal Concept, value xVsync.)                - 1.3.6.1.4.1.10098.1.1.1.19
 gotoSysStatus                  (Keeps current system status - info shown in GOsa)              - 1.3.6.1.4.1.10098.1.1.2.11
 academicTitle                  (Field to represent the academic title)                         - 1.3.6.1.4.1.10098.1.1.6.2
 gotoXKbModel                   (GOto - Gonicus Terminal Concept, value xKbmodel.)              - 1.3.6.1.4.1.10098.1.1.1.25
 fdSnapshotURI                  (FusionDirectory - Snaphost URI)                                - 1.3.6.1.4.1.38414.8.17.3
 ghMemSize                      (Hardware definitions, value memSize)                           - 1.3.6.1.4.1.10098.1.1.2.2
 gotoProfileQuota               (GOto - save quota for home)                                    - 1.3.6.1.4.1.10098.1.1.11.15
 gotoScannerClients             (GOto - Gonicus Terminal Concept, value scannerClients.)        - 1.3.6.1.4.1.10098.1.1.1.11
 fdPersonalTitleInDN            (FusionDirectory - Personal title in dn)                        - 1.3.6.1.4.1.38414.8.12.5
 fdCopyPaste                    (FusionDirectory - (de)Activate copy/paste)                     - 1.3.6.1.4.1.38414.8.14.5
 gotoFloppyEnable               (GOto - Gonicus Terminal Concept, value floppyEnable.)          - 1.3.6.1.4.1.10098.1.1.1.7
 ghIdeDev                       (Hardware definitions, value ideDev)                            - 1.3.6.1.4.1.10098.1.1.2.4
 gotoXMouseport                 (GOto - Gonicus Terminal Concept, value xMouseport.)            - 1.3.6.1.4.1.10098.1.1.1.22
 gotoXKbLayout                  (GOto - Gonicus Terminal Concept, value xKblayout.)             - 1.3.6.1.4.1.10098.1.1.1.26
 gotoAutoFs                     (GOto - Gonicus Terminal Concept, value autofs.)                - 1.3.6.1.4.1.10098.1.1.1.31
 gotoProfileFlags               (GOto - Flags for Profile handling - C is for caching)          - 1.3.6.1.4.1.10098.1.1.11.7
 gosaDefaultLanguage            (GOsa - Defines the default language for a user)                - 1.3.6.1.4.1.10098.1.1.12.14
 fdSnapshotAdminPassword        (FusionDirectory - Snaphost admin password)                     - 1.3.6.1.4.1.38414.8.17.5
 gotoHardwareChecksum           (GOto - quick way to see if something has changed)              - 1.3.6.1.4.1.10098.1.1.2.12
 gotoProfileServer              (GOto - specifies the profile server)                           - 1.3.6.1.4.1.10098.1.1.11.8
 gotoXMouseButtons              (GOto - Gonicus Terminal Concept, value xMouseButtons.)         - 1.3.6.1.4.1.10098.1.1.1.23
 gotoXHsync                     (GOto - Gonicus Terminal Concept, value xHsync.)                - 1.3.6.1.4.1.10098.1.1.1.18
 ghCpuType                      (Hardware definitions, value cpuType)                           - 1.3.6.1.4.1.10098.1.1.2.1
 gotoLpdServer                  (GOto - Gonicus Terminal Concept, value lpdServer.)             - 1.3.6.1.4.1.10098.1.1.1.4
 ghScsiDev                      (Hardware definitions, value scsiDev)                           - 1.3.6.1.4.1.10098.1.1.2.5
 ghNetNic                       (Hardware definitions, value Network Device)                    - 1.3.6.1.4.1.10098.1.1.2.8
 fdPasswordHook                 (FusionDirectory - Password hook (external command))            - 1.3.6.1.4.1.38414.8.13.4
 ghGfxAdapter                   (Hardware definitions, value Grafikkarte)                       - 1.3.6.1.4.1.10098.1.1.2.9
 gotoScannerModel               (GOto - Gonicus Terminal Concept, value scannerModel.)          - 1.3.6.1.4.1.10098.1.1.1.40
 gotoRootPasswd                 (GOto - Gonicus Terminal Concept, value rootPasswd.)            - 1.3.6.1.4.1.10098.1.1.1.14
 ghInventoryNumber              (Unique number for inclusion in an inventory)                   - 1.3.6.1.4.1.10098.1.1.2.10
 gotoCdromEnable                (GOto - Gonicus Terminal Concept, value cdromEnable.)           - 1.3.6.1.4.1.10098.1.1.1.8
 gotoModules                    (GOto - Gonicus Terminal Concept, value kernel modules.)        - 1.3.6.1.4.1.10098.1.1.1.32
 gotoXKbVariant                 (GOto - Gonicus Terminal Concept, value xKbvariant.)            - 1.3.6.1.4.1.10098.1.1.1.27
 gotoXMonitor                   (GOto - Gonicus Terminal Concept, value xMonitor.)              - 1.3.6.1.4.1.10098.1.1.1.17
 fdRfc2307bis                   (FusionDirectory - rfc2307bis)                                  - 1.3.6.1.4.1.38414.8.10.1
 gotoFontPath                   (GOto - Gonicus Terminal Concept, value fontPath.)              - 1.3.6.1.4.1.10098.1.1.1.5
 gosaSubtreeACL                 (GOsa - ACL entry)                                              - 1.3.6.1.4.1.10098.1.1.12.1
 gosaLoginRestriction           (GOsa - Multivalue attribute to carry a number of allowed ips/subnets)  - 1.3.6.1.4.1.10098.1.1.12.46
 fdIdGenerator                  (FusionDirectory - An automatic way to generate new user ids)   - 1.3.6.1.4.1.38414.8.12.4
 ghUsbSupport                   (Hardware definitions, value usbSupport)                        - 1.3.6.1.4.1.10098.1.1.2.3
 gotoAdaptPath                  (GOto - Gonicus Terminal Concept, value adaptpath.)             - 1.3.6.1.4.1.10098.1.1.1.33
 gotoFilesystem                 (GOto - Gonicus Terminal Concept, value filesystem.)            - 1.3.6.1.4.1.10098.1.1.1.6
 gotoSndModule                  (GOto - Gonicus Terminal Concept, value sound Modules.)         - 1.3.6.1.4.1.10098.1.1.1.29
 gotoScannerEnable              (GOto - Gonicus Terminal Concept, value scannerEnable.)         - 1.3.6.1.4.1.10098.1.1.1.10
 gotoXColordepth                (GOto - Gonicus Terminal Concept, value xColordepth.)           - 1.3.6.1.4.1.10098.1.1.1.21
Deprecated objectClasses:
 gosaUserTemplate               (GOsa - Class for GOsa User Templates)                          - 1.3.6.1.4.1.10098.1.2.1.19.11
 gosaAccount                    (GOsa - Class for GOsa Accounts)                                - 1.3.6.1.4.1.10098.1.2.1.19.6
 goCupsServer                   (CUPS server description)                                       - 1.3.6.1.4.1.10098.1.2.1.23
 gosaObject                     (GOsa - Class for GOsa settings)                                - 1.3.6.1.4.1.10098.1.2.1.19.1
 gosaCacheEntry                 (GOsa - Class for GOsa caching)                                 - 1.3.6.1.4.1.10098.1.2.1.19.3
```

fusiondirectory-setup --check-deprecated will output a list of dn using old attributes and objectClasses

```shell
fusiondirectory-setup --check-deprecated
List LDAP entries using deprecated attributes or objectclasses
cn=fusiondirectory,ou=configs,dc=fusiondirectory,dc=org contains an obsolete attribute
uid=fd-admin,ou=people,dc=fusiondirectory,dc=org uses the obsolete object class gosaAccount
```

fusiondirectory-setup --ldif-deprecated will output an ldif file on the console that you can use with ldapmodify to clean you ldap server from old attributes.
If they are old objectClasses it will warn you and you will have to remove it by hand, they have been specified at the fusiondirectory-setup --check-deprecated step.

```shell
fusiondirectory-setup --ldif-deprecated
# Print an LDIF removing deprecated attributes
dn:cn=fusiondirectory,ou=configs,dc=fusiondirectory,dc=org
changetype:modify
delete:fdRfc2307bis
-
delete:fdCopyPaste

# WARNING: There are entries in the LDAP using obsolete classes, you need to edit them manually
```

**Please read it carefully before applying :!: :!:**

## Migrate FusionDirectory from 1.0.9 to 1.0.9.1

### Upgrade FusionDirectory first

  * Upgrade FusionDirectory core package before other ones to avoid dependencies errors:

```shell
apt-get install fusiondirectory
```

  * Upgrade FusionDirectory schema package too.

```shell
apt-get install fusiondirectory-schema
```

### Upgrade of your LDAP directory

  * Upgrade the core template schema

```shell
fusiondirectory-insert-schema -m /etc/ldap/schema/fusiondirectory/template-fd.schema 
```

### Remove old schema from /etc/ldap/schema/fusiondirectory

The old schema are not automatically removed from /etc/ldap/schema/fusiondirectory.
You can safely remove recovery-fd.schema and asterisk-fd.conf

### Check for deprecated attributes and objectClasses in your LDAP

fusiondirectory-setup --list-deprecated show deprecated attributes and objectClasses for FusionDirectory

```shell
fusiondirectory-setup --list-deprecated
List deprecated attributes and objectclasses
Deprecated attributes:
 gotoModules                    (GOto - Gonicus Terminal Concept, value kernel modules.)        - 1.3.6.1.4.1.10098.1.1.1.32
 fdPasswordHook                 (FusionDirectory - Password hook (external command))            - 1.3.6.1.4.1.38414.8.13.4
 fdSnapshotURI                  (FusionDirectory - Snaphost URI)                                - 1.3.6.1.4.1.38414.8.17.3
 gotoXVsync                     (GOto - Gonicus Terminal Concept, value xVsync.)                - 1.3.6.1.4.1.10098.1.1.1.19
 ghSoundAdapter                 (Hardware definitions, value soundAdapter)                      - 1.3.6.1.4.1.10098.1.1.2.7
 gotoXMouseport                 (GOto - Gonicus Terminal Concept, value xMouseport.)            - 1.3.6.1.4.1.10098.1.1.1.22
 gotoXMonitor                   (GOto - Gonicus Terminal Concept, value xMonitor.)              - 1.3.6.1.4.1.10098.1.1.1.17
 gotoAdaptPath                  (GOto - Gonicus Terminal Concept, value adaptpath.)             - 1.3.6.1.4.1.10098.1.1.1.33
 gotoScannerClients             (GOto - Gonicus Terminal Concept, value scannerClients.)        - 1.3.6.1.4.1.10098.1.1.1.11
 gotoHardwareChecksum           (GOto - quick way to see if something has changed)              - 1.3.6.1.4.1.10098.1.1.2.12
 gotoRootPasswd                 (GOto - Gonicus Terminal Concept, value rootPasswd.)            - 1.3.6.1.4.1.10098.1.1.1.14
 gotoXKbLayout                  (GOto - Gonicus Terminal Concept, value xKblayout.)             - 1.3.6.1.4.1.10098.1.1.1.26
 gotoProfileServer              (GOto - specifies the profile server)                           - 1.3.6.1.4.1.10098.1.1.11.8
 fdAccountRDN                   (FusionDirectory - use a placeholder pattern for generating account RDNs)       - 1.3.6.1.4.1.38414.8.12.2
 gotoScannerEnable              (GOto - Gonicus Terminal Concept, value scannerEnable.)         - 1.3.6.1.4.1.10098.1.1.1.10
 ghGfxAdapter                   (Hardware definitions, value Grafikkarte)                       - 1.3.6.1.4.1.10098.1.1.2.9
 gotoFontPath                   (GOto - Gonicus Terminal Concept, value fontPath.)              - 1.3.6.1.4.1.10098.1.1.1.5
 ghIdeDev                       (Hardware definitions, value ideDev)                            - 1.3.6.1.4.1.10098.1.1.2.4
 gotoLpdEnable                  (GOto - Gonicus Terminal Concept, value lpdEnable.)             - 1.3.6.1.4.1.10098.1.1.1.9
 gotoXKbVariant                 (GOto - Gonicus Terminal Concept, value xKbvariant.)            - 1.3.6.1.4.1.10098.1.1.1.27
 fdRfc2307bis                   (FusionDirectory - rfc2307bis)                                  - 1.3.6.1.4.1.38414.8.10.1
 gotoAutoFs                     (GOto - Gonicus Terminal Concept, value autofs.)                - 1.3.6.1.4.1.10098.1.1.1.31
 gotoSndModule                  (GOto - Gonicus Terminal Concept, value sound Modules.)         - 1.3.6.1.4.1.10098.1.1.1.29
 gotoCdromEnable                (GOto - Gonicus Terminal Concept, value cdromEnable.)           - 1.3.6.1.4.1.10098.1.1.1.8
 gotoScannerModel               (GOto - Gonicus Terminal Concept, value scannerModel.)          - 1.3.6.1.4.1.10098.1.1.1.40
 gosaLoginRestriction           (GOsa - Multivalue attribute to carry a number of allowed ips/subnets)  - 1.3.6.1.4.1.10098.1.1.12.46
 gotoXColordepth                (GOto - Gonicus Terminal Concept, value xColordepth.)           - 1.3.6.1.4.1.10098.1.1.1.21
 academicTitle                  (Field to represent the academic title)                         - 1.3.6.1.4.1.10098.1.1.6.2
 fdSnapshotAdminDn              (FusionDirectory - Snaphost admin dn)                           - 1.3.6.1.4.1.38414.8.17.4
 gotoFilesystem                 (GOto - Gonicus Terminal Concept, value filesystem.)            - 1.3.6.1.4.1.10098.1.1.1.6
 ghInventoryNumber              (Unique number for inclusion in an inventory)                   - 1.3.6.1.4.1.10098.1.1.2.10
 gosaSubtreeACL                 (GOsa - ACL entry)                                              - 1.3.6.1.4.1.10098.1.1.12.1
 fdIdGenerator                  (FusionDirectory - An automatic way to generate new user ids)   - 1.3.6.1.4.1.38414.8.12.4
 ghUsbSupport                   (Hardware definitions, value usbSupport)                        - 1.3.6.1.4.1.10098.1.1.2.3
 gotoSysStatus                  (Keeps current system status - info shown in GOsa)              - 1.3.6.1.4.1.10098.1.1.2.11
 fdCopyPaste                    (FusionDirectory - (de)Activate copy/paste)                     - 1.3.6.1.4.1.38414.8.14.5
 gotoXDriver                    (GOto - Gonicus Terminal Concept, value xDriver.)               - 1.3.6.1.4.1.10098.1.1.1.28
 gotoXKbModel                   (GOto - Gonicus Terminal Concept, value xKbmodel.)              - 1.3.6.1.4.1.10098.1.1.1.25
 fdPersonalTitleInDN            (FusionDirectory - Personal title in dn)                        - 1.3.6.1.4.1.38414.8.12.5
 gotoLpdServer                  (GOto - Gonicus Terminal Concept, value lpdServer.)             - 1.3.6.1.4.1.10098.1.1.1.4
 gotoXHsync                     (GOto - Gonicus Terminal Concept, value xHsync.)                - 1.3.6.1.4.1.10098.1.1.1.18
 gotoProfileFlags               (GOto - Flags for Profile handling - C is for caching)          - 1.3.6.1.4.1.10098.1.1.11.7
 ghCpuType                      (Hardware definitions, value cpuType)                           - 1.3.6.1.4.1.10098.1.1.2.1
 gotoXResolution                (GOto - Gonicus Terminal Concept, value xResolution.)           - 1.3.6.1.4.1.10098.1.1.1.20
 gotoShare                      (GOto - specifies a share)                                      - 1.3.6.1.4.1.10098.1.1.11.9
 gotoScannerBackend             (GOto - Gonicus Terminal Concept, value scannerBackend.)        - 1.3.6.1.4.1.10098.1.1.1.39
 fdSnapshotAdminPassword        (FusionDirectory - Snaphost admin password)                     - 1.3.6.1.4.1.38414.8.17.5
 fdVoicemailContexts            (FusionDirectory - available voicemail contexts)                - 1.3.6.1.4.1.38414.19.11.2
 gosaDefaultLanguage            (GOsa - Defines the default language for a user)                - 1.3.6.1.4.1.10098.1.1.12.14
 ghMemSize                      (Hardware definitions, value memSize)                           - 1.3.6.1.4.1.10098.1.1.2.2
 gotoProfileQuota               (GOto - save quota for home)                                    - 1.3.6.1.4.1.10098.1.1.11.15
 fdSipContexts                  (FusionDirectory - available sip contexts)                      - 1.3.6.1.4.1.38414.19.11.1
 fdPhoneConferenceRDN           (FusionDirectory - Phone conference RDN)                        - 1.3.6.1.4.1.38414.19.10.3
 ghScsiDev                      (Hardware definitions, value scsiDev)                           - 1.3.6.1.4.1.10098.1.1.2.5
 fdPhoneMacroRDN                (FusionDirectory - Phone macro RDN)                             - 1.3.6.1.4.1.38414.19.10.2
 ghNetNic                       (Hardware definitions, value Network Device)                    - 1.3.6.1.4.1.10098.1.1.2.8
 gotoFloppyEnable               (GOto - Gonicus Terminal Concept, value floppyEnable.)          - 1.3.6.1.4.1.10098.1.1.1.7
 gotoXMouseButtons              (GOto - Gonicus Terminal Concept, value xMouseButtons.)         - 1.3.6.1.4.1.10098.1.1.1.23
 gotoXMouseType                 (Hardware definitions, value Type of mouse)                     - 1.3.6.1.4.1.10098.1.1.1.34
Deprecated objectClasses:
 goCupsServer                   (CUPS server description)                                       - 1.3.6.1.4.1.10098.1.2.1.23
 gosaCacheEntry                 (GOsa - Class for GOsa caching)                                 - 1.3.6.1.4.1.10098.1.2.1.19.3
 gosaUserTemplate               (GOsa - Class for GOsa User Templates)                          - 1.3.6.1.4.1.10098.1.2.1.19.11
 gosaAccount                    (GOsa - Class for GOsa Accounts)                                - 1.3.6.1.4.1.10098.1.2.1.19.6
 gosaObject                     (GOsa - Class for GOsa settings)                                - 1.3.6.1.4.1.10098.1.2.1.19.1
```

fusiondirectory-setup --check-deprecated will output a list of dn using old attributes and objectClasses

```shell
fusiondirectory-setup --check-deprecated
List LDAP entries using deprecated attributes or objectclasses
cn=fusiondirectory,ou=configs,dc=fusiondirectory,dc=org contains an obsolete attribute
uid=fd-admin,ou=people,dc=fusiondirectory,dc=org uses the obsolete object class gosaAccount
```

fusiondirectory-setup --migrate-users can help you migrate you user to the new core classes that are used by FusionDirectory. If they are attributes moved over to the personal plugin, 
it will automatically migrate them and you juste need to install the personal plugin

fusiondirectory-setup --ldif-deprecated will output an ldif file on the console that you can use with ldapmodify to clean you ldap server from old attributes.

If they are old objectClasses it will warn you and you will have to remove it by hand, they have been specified at the fusiondirectory-setup --check-deprecated step
at the exception of the gosaAccount class that is migrated with fusiondirectory-setup --migrate-users like mentioned above. 

Please read it carefully before applying !!


## Migrate FusionDirectory from 1.0.9.1 to 1.0.9.2

### Upgrade FusionDirectory first

  * Upgrade FusionDirectory core package before other ones to avoid dependencies errors:

```shell
apt-get install fusiondirectory
```

  * Upgrade FusionDirectory schema package too.

```shell
apt-get install fusiondirectory-schema
```

### Removed obsolete plugins

In this version we removed obsolete plugin, because they where based on software no longer existing or needed an sql backend and we decided 
they need to be rewritten using the now ldap backend of those softwares :

  * rsyslog : old plugin not maintainable anymore based on the sql backend, and we don't want to manage sql backend in FusionDiretory anymore

  * database-connector : sql abstraction code no longer needed now that rsylog plugin is removed

They are no longer supported and should be removed from your installation

```shell
apt-get remove fusiondirectory-plugin-rsyslog

apt-get remove fusiondirectory-plugin-database-connector
```

### Upgrade of your LDAP directory

  * Upgrade the core template schema

```shell
fusiondirectory-insert-schema -m /etc/ldap/schema/fusiondirectory/core-fd.schema 
```

  * if your are using the fusioninventory plugin you have to update his schema

```shell
fusiondirectory-insert-schema -m /etc/ldap/schema/fusiondirectory/inventory-fd.schema
```

  * if your are using the supann plugin you have to update his schema

```shell
fusiondirectory-insert-schema -m /etc/ldap/schema/fusiondirectory/supann-fd.conf
```

### Remove old schema from /etc/ldap/schema/fusiondirectory

The old schema are not automatically removed from /etc/ldap/schema/fusiondirectory.
You can safely remove recovery-fd.schema and asterisk-fd.conf if they still exist

### Check for deprecated attributes and objectClasses in your LDAP

fusiondirectory-setup --list-deprecated show deprecated attributes and objectClasses for FusionDirectory

```shell
fusiondirectory-setup --list-deprecated
List deprecated attributes and objectclasses
Deprecated attributes:
 gotoModules                    (GOto - Gonicus Terminal Concept, value kernel modules.)        - 1.3.6.1.4.1.10098.1.1.1.32
 fdPasswordHook                 (FusionDirectory - Password hook (external command))            - 1.3.6.1.4.1.38414.8.13.4
 fdSnapshotURI                  (FusionDirectory - Snaphost URI)                                - 1.3.6.1.4.1.38414.8.17.3
 gotoXVsync                     (GOto - Gonicus Terminal Concept, value xVsync.)                - 1.3.6.1.4.1.10098.1.1.1.19
 ghSoundAdapter                 (Hardware definitions, value soundAdapter)                      - 1.3.6.1.4.1.10098.1.1.2.7
 gotoXMouseport                 (GOto - Gonicus Terminal Concept, value xMouseport.)            - 1.3.6.1.4.1.10098.1.1.1.22
 gotoXMonitor                   (GOto - Gonicus Terminal Concept, value xMonitor.)              - 1.3.6.1.4.1.10098.1.1.1.17
 gotoAdaptPath                  (GOto - Gonicus Terminal Concept, value adaptpath.)             - 1.3.6.1.4.1.10098.1.1.1.33
 gotoScannerClients             (GOto - Gonicus Terminal Concept, value scannerClients.)        - 1.3.6.1.4.1.10098.1.1.1.11
 gotoHardwareChecksum           (GOto - quick way to see if something has changed)              - 1.3.6.1.4.1.10098.1.1.2.12
 gotoRootPasswd                 (GOto - Gonicus Terminal Concept, value rootPasswd.)            - 1.3.6.1.4.1.10098.1.1.1.14
 gotoXKbLayout                  (GOto - Gonicus Terminal Concept, value xKblayout.)             - 1.3.6.1.4.1.10098.1.1.1.26
 gotoProfileServer              (GOto - specifies the profile server)                           - 1.3.6.1.4.1.10098.1.1.11.8
 fdAccountRDN                   (FusionDirectory - use a placeholder pattern for generating account RDNs)       - 1.3.6.1.4.1.38414.8.12.2
 gotoScannerEnable              (GOto - Gonicus Terminal Concept, value scannerEnable.)         - 1.3.6.1.4.1.10098.1.1.1.10
 ghGfxAdapter                   (Hardware definitions, value Grafikkarte)                       - 1.3.6.1.4.1.10098.1.1.2.9
 gotoFontPath                   (GOto - Gonicus Terminal Concept, value fontPath.)              - 1.3.6.1.4.1.10098.1.1.1.5
 ghIdeDev                       (Hardware definitions, value ideDev)                            - 1.3.6.1.4.1.10098.1.1.2.4
 gotoLpdEnable                  (GOto - Gonicus Terminal Concept, value lpdEnable.)             - 1.3.6.1.4.1.10098.1.1.1.9
 gotoXKbVariant                 (GOto - Gonicus Terminal Concept, value xKbvariant.)            - 1.3.6.1.4.1.10098.1.1.1.27
 fdRfc2307bis                   (FusionDirectory - rfc2307bis)                                  - 1.3.6.1.4.1.38414.8.10.1
 gotoAutoFs                     (GOto - Gonicus Terminal Concept, value autofs.)                - 1.3.6.1.4.1.10098.1.1.1.31
 gotoSndModule                  (GOto - Gonicus Terminal Concept, value sound Modules.)         - 1.3.6.1.4.1.10098.1.1.1.29
 gotoCdromEnable                (GOto - Gonicus Terminal Concept, value cdromEnable.)           - 1.3.6.1.4.1.10098.1.1.1.8
 gotoScannerModel               (GOto - Gonicus Terminal Concept, value scannerModel.)          - 1.3.6.1.4.1.10098.1.1.1.40
 gosaLoginRestriction           (GOsa - Multivalue attribute to carry a number of allowed ips/subnets)  - 1.3.6.1.4.1.10098.1.1.12.46
 gotoXColordepth                (GOto - Gonicus Terminal Concept, value xColordepth.)           - 1.3.6.1.4.1.10098.1.1.1.21
 academicTitle                  (Field to represent the academic title)                         - 1.3.6.1.4.1.10098.1.1.6.2
 fdSnapshotAdminDn              (FusionDirectory - Snaphost admin dn)                           - 1.3.6.1.4.1.38414.8.17.4
 gotoFilesystem                 (GOto - Gonicus Terminal Concept, value filesystem.)            - 1.3.6.1.4.1.10098.1.1.1.6
 ghInventoryNumber              (Unique number for inclusion in an inventory)                   - 1.3.6.1.4.1.10098.1.1.2.10
 gosaSubtreeACL                 (GOsa - ACL entry)                                              - 1.3.6.1.4.1.10098.1.1.12.1
 fdIdGenerator                  (FusionDirectory - An automatic way to generate new user ids)   - 1.3.6.1.4.1.38414.8.12.4
 ghUsbSupport                   (Hardware definitions, value usbSupport)                        - 1.3.6.1.4.1.10098.1.1.2.3
 gotoSysStatus                  (Keeps current system status - info shown in GOsa)              - 1.3.6.1.4.1.10098.1.1.2.11
 fdCopyPaste                    (FusionDirectory - (de)Activate copy/paste)                     - 1.3.6.1.4.1.38414.8.14.5
 gotoXDriver                    (GOto - Gonicus Terminal Concept, value xDriver.)               - 1.3.6.1.4.1.10098.1.1.1.28
 gotoXKbModel                   (GOto - Gonicus Terminal Concept, value xKbmodel.)              - 1.3.6.1.4.1.10098.1.1.1.25
 fdPersonalTitleInDN            (FusionDirectory - Personal title in dn)                        - 1.3.6.1.4.1.38414.8.12.5
 gotoLpdServer                  (GOto - Gonicus Terminal Concept, value lpdServer.)             - 1.3.6.1.4.1.10098.1.1.1.4
 gotoXHsync                     (GOto - Gonicus Terminal Concept, value xHsync.)                - 1.3.6.1.4.1.10098.1.1.1.18
 gotoProfileFlags               (GOto - Flags for Profile handling - C is for caching)          - 1.3.6.1.4.1.10098.1.1.11.7
 ghCpuType                      (Hardware definitions, value cpuType)                           - 1.3.6.1.4.1.10098.1.1.2.1
 gotoXResolution                (GOto - Gonicus Terminal Concept, value xResolution.)           - 1.3.6.1.4.1.10098.1.1.1.20
 gotoShare                      (GOto - specifies a share)                                      - 1.3.6.1.4.1.10098.1.1.11.9
 gotoScannerBackend             (GOto - Gonicus Terminal Concept, value scannerBackend.)        - 1.3.6.1.4.1.10098.1.1.1.39
 fdSnapshotAdminPassword        (FusionDirectory - Snaphost admin password)                     - 1.3.6.1.4.1.38414.8.17.5
 fdVoicemailContexts            (FusionDirectory - available voicemail contexts)                - 1.3.6.1.4.1.38414.19.11.2
 gosaDefaultLanguage            (GOsa - Defines the default language for a user)                - 1.3.6.1.4.1.10098.1.1.12.14
 ghMemSize                      (Hardware definitions, value memSize)                           - 1.3.6.1.4.1.10098.1.1.2.2
 gotoProfileQuota               (GOto - save quota for home)                                    - 1.3.6.1.4.1.10098.1.1.11.15
 fdSipContexts                  (FusionDirectory - available sip contexts)                      - 1.3.6.1.4.1.38414.19.11.1
 fdPhoneConferenceRDN           (FusionDirectory - Phone conference RDN)                        - 1.3.6.1.4.1.38414.19.10.3
 ghScsiDev                      (Hardware definitions, value scsiDev)                           - 1.3.6.1.4.1.10098.1.1.2.5
 fdPhoneMacroRDN                (FusionDirectory - Phone macro RDN)                             - 1.3.6.1.4.1.38414.19.10.2
 ghNetNic                       (Hardware definitions, value Network Device)                    - 1.3.6.1.4.1.10098.1.1.2.8
 gotoFloppyEnable               (GOto - Gonicus Terminal Concept, value floppyEnable.)          - 1.3.6.1.4.1.10098.1.1.1.7
 gotoXMouseButtons              (GOto - Gonicus Terminal Concept, value xMouseButtons.)         - 1.3.6.1.4.1.10098.1.1.1.23
 gotoXMouseType                 (Hardware definitions, value Type of mouse)                     - 1.3.6.1.4.1.10098.1.1.1.34
Deprecated objectClasses:
 goCupsServer                   (CUPS server description)                                       - 1.3.6.1.4.1.10098.1.2.1.23
 gosaCacheEntry                 (GOsa - Class for GOsa caching)                                 - 1.3.6.1.4.1.10098.1.2.1.19.3
 gosaUserTemplate               (GOsa - Class for GOsa User Templates)                          - 1.3.6.1.4.1.10098.1.2.1.19.11
 gosaAccount                    (GOsa - Class for GOsa Accounts)                                - 1.3.6.1.4.1.10098.1.2.1.19.6
 gosaObject                     (GOsa - Class for GOsa settings)                                - 1.3.6.1.4.1.10098.1.2.1.19.1
```

fusiondirectory-setup --check-deprecated will output a list of dn using old attributes and objectClasses of they are present in your ldap server

```shell
fusiondirectory-setup --check-deprecated
List LDAP entries using deprecated attributes or objectclasses
There are no entries in the LDAP using obsolete attributes
There are no entries in the LDAP using obsolete classes
```

fusiondirectory-setup --migrate-users can help you migrate you user to the new core classes that are used by FusionDirectory. 
If they are attributes moved over to the personal plugin, it will automatically migrate them and you juste need to install the personal plugin

fusiondirectory-setup --ldif-deprecated will output an ldif file on the console that you can use with ldapmodify to clean you ldap server from old attributes.

If they are old objectClasses it will warn you and you will have to remove it by hand, they have been specified at the fusiondirectory-setup --check-deprecated 
step at the exception of the gosaAccount class that is migrated with **fusiondirectory-setup --migrate-users** like mentioned above. 

Please read it carefully before applying :!::!:

## Migrate FusionDirectory from 1.0.9.2 to 1.0.9.3

### Ubuntu 12.0.4 TLS users
 
Since 1.0.9.2 FusionDirectory need the php-cas library for CAS server support. This library can normally found in universe in the Ubuntu repositories.

In case you did not find it, grab the deb from here [php cas][php-cas] and install it

### Upgrade FusionDirectory first

  * Upgrade FusionDirectory core package before other ones to avoid dependencies errors:

```shell
apt-get install fusiondirectory
```

  * Upgrade FusionDirectory schema package too.

```shell
apt-get install fusiondirectory-schema
```

### Upgrade of your LDAP directory

  * Upgrade the core configuration schema

```shell
fusiondirectory-insert-schema -m /etc/ldap/schema/fusiondirectory/core-fd-conf.schema
```

  * if your are using the system plugin you have to update his schema

```shell
fusiondirectory-insert-schema -m /etc/ldap/schema/fusiondirectory/service-fd.schema
```

  * if your are using the repository plugin you have to update his schema

```shell
fusiondirectory-insert-schema -m /etc/ldap/schema/fusiondirectory/repository.schema
```

### Remove old schema from /etc/ldap/schema/fusiondirectory

The old schema are not automatically removed from /etc/ldap/schema/fusiondirectory.
You can safely remove **recovery-fd.schema** and **asterisk-fd.conf** if they still exist

### Check for deprecated attributes and objectClasses in your LDAP

**fusiondirectory-setup --list-deprecated** show deprecated attributes and objectClasses for FusionDirectory

```shell
fusiondirectory-setup --list-deprecated
List deprecated attributes and objectclasses
Deprecated attributes:
 gotoModules                    (GOto - Gonicus Terminal Concept, value kernel modules.)        - 1.3.6.1.4.1.10098.1.1.1.32
 fdPasswordHook                 (FusionDirectory - Password hook (external command))            - 1.3.6.1.4.1.38414.8.13.4
 fdSnapshotURI                  (FusionDirectory - Snaphost URI)                                - 1.3.6.1.4.1.38414.8.17.3
 gotoXVsync                     (GOto - Gonicus Terminal Concept, value xVsync.)                - 1.3.6.1.4.1.10098.1.1.1.19
 ghSoundAdapter                 (Hardware definitions, value soundAdapter)                      - 1.3.6.1.4.1.10098.1.1.2.7
 gotoXMouseport                 (GOto - Gonicus Terminal Concept, value xMouseport.)            - 1.3.6.1.4.1.10098.1.1.1.22
 gotoXMonitor                   (GOto - Gonicus Terminal Concept, value xMonitor.)              - 1.3.6.1.4.1.10098.1.1.1.17
 gotoAdaptPath                  (GOto - Gonicus Terminal Concept, value adaptpath.)             - 1.3.6.1.4.1.10098.1.1.1.33
 gotoScannerClients             (GOto - Gonicus Terminal Concept, value scannerClients.)        - 1.3.6.1.4.1.10098.1.1.1.11
 gotoHardwareChecksum           (GOto - quick way to see if something has changed)              - 1.3.6.1.4.1.10098.1.1.2.12
 gotoRootPasswd                 (GOto - Gonicus Terminal Concept, value rootPasswd.)            - 1.3.6.1.4.1.10098.1.1.1.14
 gotoXKbLayout                  (GOto - Gonicus Terminal Concept, value xKblayout.)             - 1.3.6.1.4.1.10098.1.1.1.26
 gotoProfileServer              (GOto - specifies the profile server)                           - 1.3.6.1.4.1.10098.1.1.11.8
 fdAccountRDN                   (FusionDirectory - use a placeholder pattern for generating account RDNs)       - 1.3.6.1.4.1.38414.8.12.2
 gotoScannerEnable              (GOto - Gonicus Terminal Concept, value scannerEnable.)         - 1.3.6.1.4.1.10098.1.1.1.10
 ghGfxAdapter                   (Hardware definitions, value Grafikkarte)                       - 1.3.6.1.4.1.10098.1.1.2.9
 gotoFontPath                   (GOto - Gonicus Terminal Concept, value fontPath.)              - 1.3.6.1.4.1.10098.1.1.1.5
 ghIdeDev                       (Hardware definitions, value ideDev)                            - 1.3.6.1.4.1.10098.1.1.2.4
 gotoLpdEnable                  (GOto - Gonicus Terminal Concept, value lpdEnable.)             - 1.3.6.1.4.1.10098.1.1.1.9
 gotoXKbVariant                 (GOto - Gonicus Terminal Concept, value xKbvariant.)            - 1.3.6.1.4.1.10098.1.1.1.27
 fdRfc2307bis                   (FusionDirectory - rfc2307bis)                                  - 1.3.6.1.4.1.38414.8.10.1
 gotoAutoFs                     (GOto - Gonicus Terminal Concept, value autofs.)                - 1.3.6.1.4.1.10098.1.1.1.31
 gotoSndModule                  (GOto - Gonicus Terminal Concept, value sound Modules.)         - 1.3.6.1.4.1.10098.1.1.1.29
 gotoCdromEnable                (GOto - Gonicus Terminal Concept, value cdromEnable.)           - 1.3.6.1.4.1.10098.1.1.1.8
 gotoScannerModel               (GOto - Gonicus Terminal Concept, value scannerModel.)          - 1.3.6.1.4.1.10098.1.1.1.40
 gosaLoginRestriction           (GOsa - Multivalue attribute to carry a number of allowed ips/subnets)  - 1.3.6.1.4.1.10098.1.1.12.46
 gotoXColordepth                (GOto - Gonicus Terminal Concept, value xColordepth.)           - 1.3.6.1.4.1.10098.1.1.1.21
 academicTitle                  (Field to represent the academic title)                         - 1.3.6.1.4.1.10098.1.1.6.2
 fdSnapshotAdminDn              (FusionDirectory - Snaphost admin dn)                           - 1.3.6.1.4.1.38414.8.17.4
 gotoFilesystem                 (GOto - Gonicus Terminal Concept, value filesystem.)            - 1.3.6.1.4.1.10098.1.1.1.6
 ghInventoryNumber              (Unique number for inclusion in an inventory)                   - 1.3.6.1.4.1.10098.1.1.2.10
 gosaSubtreeACL                 (GOsa - ACL entry)                                              - 1.3.6.1.4.1.10098.1.1.12.1
 fdIdGenerator                  (FusionDirectory - An automatic way to generate new user ids)   - 1.3.6.1.4.1.38414.8.12.4
 ghUsbSupport                   (Hardware definitions, value usbSupport)                        - 1.3.6.1.4.1.10098.1.1.2.3
 gotoSysStatus                  (Keeps current system status - info shown in GOsa)              - 1.3.6.1.4.1.10098.1.1.2.11
 fdCopyPaste                    (FusionDirectory - (de)Activate copy/paste)                     - 1.3.6.1.4.1.38414.8.14.5
 gotoXDriver                    (GOto - Gonicus Terminal Concept, value xDriver.)               - 1.3.6.1.4.1.10098.1.1.1.28
 gotoXKbModel                   (GOto - Gonicus Terminal Concept, value xKbmodel.)              - 1.3.6.1.4.1.10098.1.1.1.25
 fdPersonalTitleInDN            (FusionDirectory - Personal title in dn)                        - 1.3.6.1.4.1.38414.8.12.5
 gotoLpdServer                  (GOto - Gonicus Terminal Concept, value lpdServer.)             - 1.3.6.1.4.1.10098.1.1.1.4
 gotoXHsync                     (GOto - Gonicus Terminal Concept, value xHsync.)                - 1.3.6.1.4.1.10098.1.1.1.18
 gotoProfileFlags               (GOto - Flags for Profile handling - C is for caching)          - 1.3.6.1.4.1.10098.1.1.11.7
 ghCpuType                      (Hardware definitions, value cpuType)                           - 1.3.6.1.4.1.10098.1.1.2.1
 gotoXResolution                (GOto - Gonicus Terminal Concept, value xResolution.)           - 1.3.6.1.4.1.10098.1.1.1.20
 gotoShare                      (GOto - specifies a share)                                      - 1.3.6.1.4.1.10098.1.1.11.9
 gotoScannerBackend             (GOto - Gonicus Terminal Concept, value scannerBackend.)        - 1.3.6.1.4.1.10098.1.1.1.39
 fdSnapshotAdminPassword        (FusionDirectory - Snaphost admin password)                     - 1.3.6.1.4.1.38414.8.17.5
 fdVoicemailContexts            (FusionDirectory - available voicemail contexts)                - 1.3.6.1.4.1.38414.19.11.2
 gosaDefaultLanguage            (GOsa - Defines the default language for a user)                - 1.3.6.1.4.1.10098.1.1.12.14
 ghMemSize                      (Hardware definitions, value memSize)                           - 1.3.6.1.4.1.10098.1.1.2.2
 gotoProfileQuota               (GOto - save quota for home)                                    - 1.3.6.1.4.1.10098.1.1.11.15
 fdSipContexts                  (FusionDirectory - available sip contexts)                      - 1.3.6.1.4.1.38414.19.11.1
 fdPhoneConferenceRDN           (FusionDirectory - Phone conference RDN)                        - 1.3.6.1.4.1.38414.19.10.3
 ghScsiDev                      (Hardware definitions, value scsiDev)                           - 1.3.6.1.4.1.10098.1.1.2.5
 fdPhoneMacroRDN                (FusionDirectory - Phone macro RDN)                             - 1.3.6.1.4.1.38414.19.10.2
 ghNetNic                       (Hardware definitions, value Network Device)                    - 1.3.6.1.4.1.10098.1.1.2.8
 gotoFloppyEnable               (GOto - Gonicus Terminal Concept, value floppyEnable.)          - 1.3.6.1.4.1.10098.1.1.1.7
 gotoXMouseButtons              (GOto - Gonicus Terminal Concept, value xMouseButtons.)         - 1.3.6.1.4.1.10098.1.1.1.23
 gotoXMouseType                 (Hardware definitions, value Type of mouse)                     - 1.3.6.1.4.1.10098.1.1.1.34
Deprecated objectClasses:
 goCupsServer                   (CUPS server description)                                       - 1.3.6.1.4.1.10098.1.2.1.23
 gosaCacheEntry                 (GOsa - Class for GOsa caching)                                 - 1.3.6.1.4.1.10098.1.2.1.19.3
 gosaUserTemplate               (GOsa - Class for GOsa User Templates)                          - 1.3.6.1.4.1.10098.1.2.1.19.11
 gosaAccount                    (GOsa - Class for GOsa Accounts)                                - 1.3.6.1.4.1.10098.1.2.1.19.6
 gosaObject                     (GOsa - Class for GOsa settings)                                - 1.3.6.1.4.1.10098.1.2.1.19.1
```

fusiondirectory-setup --check-deprecated will output a list of dn using old attributes and objectClasses of they are present in your ldap server

```shell
fusiondirectory-setup --check-deprecated
List LDAP entries using deprecated attributes or objectclasses
There are no entries in the LDAP using obsolete attributes
There are no entries in the LDAP using obsolete classes
```

fusiondirectory-setup --migrate-users can help you migrate you user to the new core classes that are used by FusionDirectory. 
If they are attributes moved over to the personal plugin, it will automatically migrate them and you juste need to install the personal plugin

fusiondirectory-setup --ldif-deprecated will output an ldif file on the console that you can use with ldapmodify to clean you ldap server from old attributes.

If they are old objectClasses it will warn you and you will have to remove it by hand, they have been specified at the fusiondirectory-setup --check-deprecated 
step at the exception of the gosaAccount class that is migrated with **fusiondirectory-setup --migrate-users** like mentioned above. 

Please read it carefully before applying !!

## Migrate FusionDirectory from 1.0.9.3 to 1.0.10

### Ubuntu 12.0.4 TLS users
 
Since 1.0.9.2 FusionDirectory need the php-cas library for CAS server support. This library can normally found in universe in the Ubuntu repositories.

In case you did not find it, grab the deb from here [php cas][php-cas] and install it

### Upgrade FusionDirectory first

  * Upgrade FusionDirectory core package before other ones to avoid dependencies errors:

```shell
apt-get install fusiondirectory
```

  * Upgrade FusionDirectory schema package too.

```shell
apt-get install fusiondirectory-schema
```

### Upgrade of your LDAP directory

  * Upgrade the core configuration schema

```shell
fusiondirectory-insert-schema -m /etc/ldap/schema/fusiondirectory/core-fd.schema

fusiondirectory-insert-schema -m /etc/ldap/schema/fusiondirectory/core-fd-conf.schema
```

  * if your are using the personal plugin you have to update his schema

```shell
fusiondirectory-insert-schema -m /etc/ldap/schema/fusiondirectory/personal-fd.schema
```

- if your are using the system plugin you have to update his schema

```shell
fusiondirectory-insert-schema -m /etc/ldap/schema/fusiondirectory/systems-fd.schema

fusiondirectory-insert-schema -m /etc/ldap/schema/fusiondirectory/systems-fd-conf.schema 

fusiondirectory-insert-schema -m /etc/ldap/schema/fusiondirectory/service-fd.schema
```

- if your are using the dns plugin you have to update his schema

```shell
fusiondirectory-insert-schema -m /etc/ldap/schema/fusiondirectory/dns-fd-conf.schema 
```

and install a new schema

```shell
fusiondirectory-insert-schema -m /etc/ldap/schema/fusiondirectory/dns-fd.schema 
```

### Remove old schema from /etc/ldap/schema/fusiondirectory

The old schema are not automatically removed from /etc/ldap/schema/fusiondirectory.
You can safely remove **recovery-fd.schema** and **asterisk-fd-conf.schema** if they still exist

### Check for deprecated attributes and objectClasses in your LDAP

fusiondirectory-setup --list-deprecated show deprecated attributes and objectClasses for FusionDirectory

```shell
fusiondirectory-setup --list-deprecated
List deprecated attributes and objectclasses
Deprecated attributes:
 gotoFilesystem                 (GOto - Gonicus Terminal Concept, value filesystem.)            - 1.3.6.1.4.1.10098.1.1.1.6
 gotoFloppyEnable               (GOto - Gonicus Terminal Concept, value floppyEnable.)          - 1.3.6.1.4.1.10098.1.1.1.7
 gotoSysStatus                  (Keeps current system status - info shown in GOsa)              - 1.3.6.1.4.1.10098.1.1.2.11
 gotoXColordepth                (GOto - Gonicus Terminal Concept, value xColordepth.)           - 1.3.6.1.4.1.10098.1.1.1.21
 gotoHardwareChecksum           (GOto - quick way to see if something has changed)              - 1.3.6.1.4.1.10098.1.1.2.12
 goFaxPassword                  (Admin password for fax server)                                 - 1.3.6.1.4.1.10098.1.1.9.23
 gotoNtpServer                  (GOto - Gonicus Terminal Concept, value ntpServer.)             - 1.3.6.1.4.1.10098.1.1.1.2
 gotoXDriver                    (GOto - Gonicus Terminal Concept, value xDriver.)               - 1.3.6.1.4.1.10098.1.1.1.28
 fdSnapshotAdminDn              (FusionDirectory - Snaphost admin dn)                           - 1.3.6.1.4.1.38414.8.17.4
 fdPersonalTitleInDN            (FusionDirectory - Personal title in dn)                        - 1.3.6.1.4.1.38414.8.12.5
 gotoXMouseport                 (GOto - Gonicus Terminal Concept, value xMouseport.)            - 1.3.6.1.4.1.10098.1.1.1.22
 gotoAdaptPath                  (GOto - Gonicus Terminal Concept, value adaptpath.)             - 1.3.6.1.4.1.10098.1.1.1.33
 goFonCountryCode               (Store country code)                                            - 1.3.6.1.4.1.10098.1.1.9.29
 gotoSndModule                  (GOto - Gonicus Terminal Concept, value sound Modules.)         - 1.3.6.1.4.1.10098.1.1.1.29
 gotoXKbLayout                  (GOto - Gonicus Terminal Concept, value xKblayout.)             - 1.3.6.1.4.1.10098.1.1.1.26
 gotoProfileFlags               (GOto - Flags for Profile handling - C is for caching)          - 1.3.6.1.4.1.10098.1.1.11.7
 ghSoundAdapter                 (Hardware definitions, value soundAdapter)                      - 1.3.6.1.4.1.10098.1.1.2.7
 gotoScannerModel               (GOto - Gonicus Terminal Concept, value scannerModel.)          - 1.3.6.1.4.1.10098.1.1.1.40
 gotoScannerEnable              (GOto - Gonicus Terminal Concept, value scannerEnable.)         - 1.3.6.1.4.1.10098.1.1.1.10
 gosaSubtreeACL                 (GOsa - ACL entry)                                              - 1.3.6.1.4.1.10098.1.1.12.1
 ghUsbSupport                   (Hardware definitions, value usbSupport)                        - 1.3.6.1.4.1.10098.1.1.2.3
 gotoLpdEnable                  (GOto - Gonicus Terminal Concept, value lpdEnable.)             - 1.3.6.1.4.1.10098.1.1.1.9
 gotoProfileQuota               (GOto - save quota for home)                                    - 1.3.6.1.4.1.10098.1.1.11.15
 gotoScannerBackend             (GOto - Gonicus Terminal Concept, value scannerBackend.)        - 1.3.6.1.4.1.10098.1.1.1.39
 goSyslogSection                (What sections wants the server for its syslog service? i.e. *.*)       - 1.3.6.1.4.1.10098.1.1.9.9
 fdAccountRDN                   (FusionDirectory - use a placeholder pattern for generating account RDNs)       - 1.3.6.1.4.1.38414.8.12.2
 goFaxAdmin                     (Admin principal for fax server)                                - 1.3.6.1.4.1.10098.1.1.9.22
 goLogDriver                    (FD logging MDB2 driver name)                                   - 1.3.6.1.4.1.10098.1.1.9.84
 ghNetNic                       (Hardware definitions, value Network Device)                    - 1.3.6.1.4.1.10098.1.1.2.8
 academicTitle                  (Field to represent the academic title)                         - 1.3.6.1.4.1.10098.1.1.6.2
 fdPhoneMacroRDN                (FusionDirectory - Phone macro RDN)                             - 1.3.6.1.4.1.38414.19.10.2
 gosaLoginRestriction           (GOsa - Multivalue attribute to carry a number of allowed ips/subnets)  - 1.3.6.1.4.1.10098.1.1.12.46
 gotoLpdServer                  (GOto - Gonicus Terminal Concept, value lpdServer.)             - 1.3.6.1.4.1.10098.1.1.1.4
 ghCpuType                      (Hardware definitions, value cpuType)                           - 1.3.6.1.4.1.10098.1.1.2.1
 gotoXMonitor                   (GOto - Gonicus Terminal Concept, value xMonitor.)              - 1.3.6.1.4.1.10098.1.1.1.17
 fdRfc2307bis                   (FusionDirectory - rfc2307bis)                                  - 1.3.6.1.4.1.38414.8.10.1
 gotoXVsync                     (GOto - Gonicus Terminal Concept, value xVsync.)                - 1.3.6.1.4.1.10098.1.1.1.19
 fdSipContexts                  (FusionDirectory - available sip contexts)                      - 1.3.6.1.4.1.38414.19.11.1
 gotoShare                      (GOto - specifies a share)                                      - 1.3.6.1.4.1.10098.1.1.11.9
 fdCopyPaste                    (FusionDirectory - (de)Activate copy/paste)                     - 1.3.6.1.4.1.38414.8.14.5
 goLogPassword                  (Admin password for log server)                                 - 1.3.6.1.4.1.10098.1.1.9.25
 gotoCdromEnable                (GOto - Gonicus Terminal Concept, value cdromEnable.)           - 1.3.6.1.4.1.10098.1.1.1.8
 gotoProfileServer              (GOto - specifies the profile server)                           - 1.3.6.1.4.1.10098.1.1.11.8
 fdSnapshotAdminPassword        (FusionDirectory - Snaphost admin password)                     - 1.3.6.1.4.1.38414.8.17.5
 ghMemSize                      (Hardware definitions, value memSize)                           - 1.3.6.1.4.1.10098.1.1.2.2
 fdSnapshotURI                  (FusionDirectory - Snaphost URI)                                - 1.3.6.1.4.1.38414.8.17.3
 gotoAutoFs                     (GOto - Gonicus Terminal Concept, value autofs.)                - 1.3.6.1.4.1.10098.1.1.1.31
 goFonAreaCode                  (Store area code)                                               - 1.3.6.1.4.1.10098.1.1.9.28
 ghInventoryNumber              (Unique number for inclusion in an inventory)                   - 1.3.6.1.4.1.10098.1.1.2.10
 fdPhoneConferenceRDN           (FusionDirectory - Phone conference RDN)                        - 1.3.6.1.4.1.38414.19.10.3
 gotoXMouseType                 (Hardware definitions, value Type of mouse)                     - 1.3.6.1.4.1.10098.1.1.1.34
 fdPrimaryGroupFilter           (FusionDirectory - Primary group filter)                        - 1.3.6.1.4.1.38414.8.14.1
 gotoXHsync                     (GOto - Gonicus Terminal Concept, value xHsync.)                - 1.3.6.1.4.1.10098.1.1.1.18
 gosaDefaultLanguage            (GOsa - Defines the default language for a user)                - 1.3.6.1.4.1.10098.1.1.12.14
 ghIdeDev                       (Hardware definitions, value ideDev)                            - 1.3.6.1.4.1.10098.1.1.2.4
 gotoXKbVariant                 (GOto - Gonicus Terminal Concept, value xKbvariant.)            - 1.3.6.1.4.1.10098.1.1.1.27
 gotoModules                    (GOto - Gonicus Terminal Concept, value kernel modules.)        - 1.3.6.1.4.1.10098.1.1.1.32
 gotoXMouseButtons              (GOto - Gonicus Terminal Concept, value xMouseButtons.)         - 1.3.6.1.4.1.10098.1.1.1.23
 fdIdGenerator                  (FusionDirectory - An automatic way to generate new user ids)   - 1.3.6.1.4.1.38414.8.12.4
 fdVoicemailContexts            (FusionDirectory - available voicemail contexts)                - 1.3.6.1.4.1.38414.19.11.2
 ghScsiDev                      (Hardware definitions, value scsiDev)                           - 1.3.6.1.4.1.10098.1.1.2.5
 goFonAdmin                     (Admin user for fon server)                                     - 1.3.6.1.4.1.10098.1.1.9.26
 ghGfxAdapter                   (Hardware definitions, value Grafikkarte)                       - 1.3.6.1.4.1.10098.1.1.2.9
 fdPasswordHook                 (FusionDirectory - Password hook (external command))            - 1.3.6.1.4.1.38414.8.13.4
 gotoXKbModel                   (GOto - Gonicus Terminal Concept, value xKbmodel.)              - 1.3.6.1.4.1.10098.1.1.1.25
 gotoRootPasswd                 (GOto - Gonicus Terminal Concept, value rootPasswd.)            - 1.3.6.1.4.1.10098.1.1.1.14
 goLogDB                        (GOsa logging DB name)                                          - 1.3.6.1.4.1.10098.1.1.9.83
 goLogAdmin                     (Admin user for log server)                                     - 1.3.6.1.4.1.10098.1.1.9.24
 gotoXResolution                (GOto - Gonicus Terminal Concept, value xResolution.)           - 1.3.6.1.4.1.10098.1.1.1.20
 gotoScannerClients             (GOto - Gonicus Terminal Concept, value scannerClients.)        - 1.3.6.1.4.1.10098.1.1.1.11
 gotoFontPath                   (GOto - Gonicus Terminal Concept, value fontPath.)              - 1.3.6.1.4.1.10098.1.1.1.5
 goFonPassword                  (Admin password for fon server)                                 - 1.3.6.1.4.1.10098.1.1.9.27
Deprecated objectClasses:
 goNfsServer                    (NFS server description)                                        - 1.3.6.1.4.1.10098.1.2.1.19
 goCupsServer                   (CUPS server description)                                       - 1.3.6.1.4.1.10098.1.2.1.23
 gosaCacheEntry                 (GOsa - Class for GOsa caching)                                 - 1.3.6.1.4.1.10098.1.2.1.19.3
 gosaObject                     (GOsa - Class for GOsa settings)                                - 1.3.6.1.4.1.10098.1.2.1.19.1
 goFaxServer                    (Fax server description)                                        - 1.3.6.1.4.1.10098.1.2.1.26
 goFonServer                    (Fon server description)                                        - 1.3.6.1.4.1.10098.1.2.1.29
 gosaUserTemplate               (GOsa - Class for GOsa User Templates)                          - 1.3.6.1.4.1.10098.1.2.1.19.11
 gosaAccount                    (GOsa - Class for GOsa Accounts)                                - 1.3.6.1.4.1.10098.1.2.1.19.6
 goLogDBServer                  (Log DB server description)                                     - 1.3.6.1.4.1.10098.1.2.1.28
 goSyslogServer                 (Syslog server description)                                     - 1.3.6.1.4.1.10098.1.2.1.21
 goNtpServer                    (Time server description)                                       - 1.3.6.1.4.1.10098.1.2.1.20
```

fusiondirectory-setup --check-deprecated will output a list of dn using old attributes and objectClasses of they are present in your ldap server

```shell
fusiondirectory-setup --check-deprecated
List LDAP entries using deprecated attributes or objectclasses
There are no entries in the LDAP using obsolete attributes
There are no entries in the LDAP using obsolete classes
```

fusiondirectory-setup --ldif-deprecated will output an ldif file on the console that you can use with ldapmodify to clean you ldap server from old attributes.

If they are old objectClasses it will warn you and you will have to remove it by hand, they have been specified at the **fusiondirectory-setup --check-deprecated** step.

!! Please read it carefully before applying !!

## Migrate FusionDirectory from 1.0.10 to 1.0.11

### Ubuntu 12.0.4 TLS users
 
Since 1.0.9.2 FusionDirectory need the php-cas library for CAS server support. This library can normally found in universe in the Ubuntu repositories.

In case you did not find it, grab the deb from here [php cas][php-cas] and install it

### Upgrade FusionDirectory first

  * Upgrade FusionDirectory core package before other ones to avoid dependencies errors:

```shell
apt-get install fusiondirectory
```

  * Upgrade FusionDirectory schema package too.

```shell
apt-get install fusiondirectory-schema
```


### Migrate FusionDirectory from 1.0.11 to 1.0.12

### Ubuntu 12.0.4 TLS users
 
Since 1.0.9.2 FusionDirectory need the php-cas library for CAS server support. This library can normally found in universe in the Ubuntu repositories.

In case you did not find it, grab the deb from here [php cas][php-cas] and install it

### Mail plugin

In this version the mail plugin has been cleaned and his dependency to the systems has been removed. 
In the mean time the postfix and spamassassin service have been moved in their own plugins. The antivirus service have been removed.

The old spamassasin settings from the mail / user tab have been rewritten and moved to a spamassasin, they now support the real LDAP backend of spamassasin. 
So you will need to move your data to the new format before using them.

### Upgrade FusionDirectory first

  * Upgrade FusionDirectory core package before other ones to avoid dependencies errors:

```shell
apt-get install fusiondirectory
```

  * Upgrade FusionDirectory schema package too.

```shell
apt-get install fusiondirectory-schema
```

### Upgrade of your LDAP directory

  * if your are using the mail plugin you have to update its schema

```shell
fusiondirectory-insert-schema -m /etc/ldap/schema/fusiondirectory/mail-fd.schema

fusiondirectory-insert-schema -m /etc/ldap/schema/fusiondirectory/mail-fd-conf.schema
```

  * if your are using the system plugin you have to update its schemas

```shell
fusiondirectory-insert-schema -m /etc/ldap/schema/fusiondirectory/service-fd.schema
```

### Postfix plugin

  * if you where using the postfix service in 1.0.11, you need to install the postfix plugin and insert the postfix schema

```shell
apt-get install fusiondirectory-plugin-postfix

apt-get install fusiondirectory-plugin-postfix-schema

fusiondirectory-insert-schema -m /etc/ldap/schema/fusiondirectory/postfix-fd.schema
```

### Spamassasin plugin

  * if you where using the spamassin service in 1.0.11, you need to install the spamassassin plugin and insert the spamassasin schema

```shell
apt-get install fusiondirectory-plugin-spamassassin

apt-get install fusiondirectory-plugin-spamassassin-schema

fusiondirectory-insert-schema -m /etc/ldap/schema/fusiondirectory/spamassassin-fd.schema
```

### Remove old schema from /etc/ldap/schema/fusiondirectory

The old schema are not automatically removed from /etc/ldap/schema/fusiondirectory.
You can safely remove **recovery-fd.schema** and **asterisk-fd-conf.schema** if they still exist

### Check for deprecated attributes and objectClasses in your LDAP

fusiondirectory-setup --list-deprecated show deprecated attributes and objectClasses for FusionDirectory

```shell
fusiondirectory-setup --list-deprecated
List deprecated attributes and objectclasses
Deprecated attributes:
 gotoFilesystem                 (GOto - Gonicus Terminal Concept, value filesystem.)            - 1.3.6.1.4.1.10098.1.1.1.6
 gotoFloppyEnable               (GOto - Gonicus Terminal Concept, value floppyEnable.)          - 1.3.6.1.4.1.10098.1.1.1.7
 gotoSysStatus                  (Keeps current system status - info shown in GOsa)              - 1.3.6.1.4.1.10098.1.1.2.11
 gotoXColordepth                (GOto - Gonicus Terminal Concept, value xColordepth.)           - 1.3.6.1.4.1.10098.1.1.1.21
 gotoHardwareChecksum           (GOto - quick way to see if something has changed)              - 1.3.6.1.4.1.10098.1.1.2.12
 goFaxPassword                  (Admin password for fax server)                                 - 1.3.6.1.4.1.10098.1.1.9.23
 gotoNtpServer                  (GOto - Gonicus Terminal Concept, value ntpServer.)             - 1.3.6.1.4.1.10098.1.1.1.2
 gotoXDriver                    (GOto - Gonicus Terminal Concept, value xDriver.)               - 1.3.6.1.4.1.10098.1.1.1.28
 fdSnapshotAdminDn              (FusionDirectory - Snaphost admin dn)                           - 1.3.6.1.4.1.38414.8.17.4
 fdPersonalTitleInDN            (FusionDirectory - Personal title in dn)                        - 1.3.6.1.4.1.38414.8.12.5
 gotoXMouseport                 (GOto - Gonicus Terminal Concept, value xMouseport.)            - 1.3.6.1.4.1.10098.1.1.1.22
 gotoAdaptPath                  (GOto - Gonicus Terminal Concept, value adaptpath.)             - 1.3.6.1.4.1.10098.1.1.1.33
 goFonCountryCode               (Store country code)                                            - 1.3.6.1.4.1.10098.1.1.9.29
 gotoSndModule                  (GOto - Gonicus Terminal Concept, value sound Modules.)         - 1.3.6.1.4.1.10098.1.1.1.29
 gotoXKbLayout                  (GOto - Gonicus Terminal Concept, value xKblayout.)             - 1.3.6.1.4.1.10098.1.1.1.26
 gotoProfileFlags               (GOto - Flags for Profile handling - C is for caching)          - 1.3.6.1.4.1.10098.1.1.11.7
 ghSoundAdapter                 (Hardware definitions, value soundAdapter)                      - 1.3.6.1.4.1.10098.1.1.2.7
 gotoScannerModel               (GOto - Gonicus Terminal Concept, value scannerModel.)          - 1.3.6.1.4.1.10098.1.1.1.40
 gotoScannerEnable              (GOto - Gonicus Terminal Concept, value scannerEnable.)         - 1.3.6.1.4.1.10098.1.1.1.10
 gosaSubtreeACL                 (GOsa - ACL entry)                                              - 1.3.6.1.4.1.10098.1.1.12.1
 ghUsbSupport                   (Hardware definitions, value usbSupport)                        - 1.3.6.1.4.1.10098.1.1.2.3
 gotoLpdEnable                  (GOto - Gonicus Terminal Concept, value lpdEnable.)             - 1.3.6.1.4.1.10098.1.1.1.9
 gotoProfileQuota               (GOto - save quota for home)                                    - 1.3.6.1.4.1.10098.1.1.11.15
 gotoScannerBackend             (GOto - Gonicus Terminal Concept, value scannerBackend.)        - 1.3.6.1.4.1.10098.1.1.1.39
 goSyslogSection                (What sections wants the server for its syslog service? i.e. *.*)       - 1.3.6.1.4.1.10098.1.1.9.9
 fdAccountRDN                   (FusionDirectory - use a placeholder pattern for generating account RDNs)       - 1.3.6.1.4.1.38414.8.12.2
 goFaxAdmin                     (Admin principal for fax server)                                - 1.3.6.1.4.1.10098.1.1.9.22
 goLogDriver                    (FD logging MDB2 driver name)                                   - 1.3.6.1.4.1.10098.1.1.9.84
 ghNetNic                       (Hardware definitions, value Network Device)                    - 1.3.6.1.4.1.10098.1.1.2.8
 academicTitle                  (Field to represent the academic title)                         - 1.3.6.1.4.1.10098.1.1.6.2
 fdPhoneMacroRDN                (FusionDirectory - Phone macro RDN)                             - 1.3.6.1.4.1.38414.19.10.2
 gosaLoginRestriction           (GOsa - Multivalue attribute to carry a number of allowed ips/subnets)  - 1.3.6.1.4.1.10098.1.1.12.46
 gotoLpdServer                  (GOto - Gonicus Terminal Concept, value lpdServer.)             - 1.3.6.1.4.1.10098.1.1.1.4
 ghCpuType                      (Hardware definitions, value cpuType)                           - 1.3.6.1.4.1.10098.1.1.2.1
 gotoXMonitor                   (GOto - Gonicus Terminal Concept, value xMonitor.)              - 1.3.6.1.4.1.10098.1.1.1.17
 fdRfc2307bis                   (FusionDirectory - rfc2307bis)                                  - 1.3.6.1.4.1.38414.8.10.1
 gotoXVsync                     (GOto - Gonicus Terminal Concept, value xVsync.)                - 1.3.6.1.4.1.10098.1.1.1.19
 fdSipContexts                  (FusionDirectory - available sip contexts)                      - 1.3.6.1.4.1.38414.19.11.1
 gotoShare                      (GOto - specifies a share)                                      - 1.3.6.1.4.1.10098.1.1.11.9
 fdCopyPaste                    (FusionDirectory - (de)Activate copy/paste)                     - 1.3.6.1.4.1.38414.8.14.5
 goLogPassword                  (Admin password for log server)                                 - 1.3.6.1.4.1.10098.1.1.9.25
 gotoCdromEnable                (GOto - Gonicus Terminal Concept, value cdromEnable.)           - 1.3.6.1.4.1.10098.1.1.1.8
 gotoProfileServer              (GOto - specifies the profile server)                           - 1.3.6.1.4.1.10098.1.1.11.8
 fdSnapshotAdminPassword        (FusionDirectory - Snaphost admin password)                     - 1.3.6.1.4.1.38414.8.17.5
 ghMemSize                      (Hardware definitions, value memSize)                           - 1.3.6.1.4.1.10098.1.1.2.2
 fdSnapshotURI                  (FusionDirectory - Snaphost URI)                                - 1.3.6.1.4.1.38414.8.17.3
 gotoAutoFs                     (GOto - Gonicus Terminal Concept, value autofs.)                - 1.3.6.1.4.1.10098.1.1.1.31
 goFonAreaCode                  (Store area code)                                               - 1.3.6.1.4.1.10098.1.1.9.28
 ghInventoryNumber              (Unique number for inclusion in an inventory)                   - 1.3.6.1.4.1.10098.1.1.2.10
 fdPhoneConferenceRDN           (FusionDirectory - Phone conference RDN)                        - 1.3.6.1.4.1.38414.19.10.3
 gotoXMouseType                 (Hardware definitions, value Type of mouse)                     - 1.3.6.1.4.1.10098.1.1.1.34
 fdPrimaryGroupFilter           (FusionDirectory - Primary group filter)                        - 1.3.6.1.4.1.38414.8.14.1
 gotoXHsync                     (GOto - Gonicus Terminal Concept, value xHsync.)                - 1.3.6.1.4.1.10098.1.1.1.18
 gosaDefaultLanguage            (GOsa - Defines the default language for a user)                - 1.3.6.1.4.1.10098.1.1.12.14
 ghIdeDev                       (Hardware definitions, value ideDev)                            - 1.3.6.1.4.1.10098.1.1.2.4
 gotoXKbVariant                 (GOto - Gonicus Terminal Concept, value xKbvariant.)            - 1.3.6.1.4.1.10098.1.1.1.27
 gotoModules                    (GOto - Gonicus Terminal Concept, value kernel modules.)        - 1.3.6.1.4.1.10098.1.1.1.32
 gotoXMouseButtons              (GOto - Gonicus Terminal Concept, value xMouseButtons.)         - 1.3.6.1.4.1.10098.1.1.1.23
 fdIdGenerator                  (FusionDirectory - An automatic way to generate new user ids)   - 1.3.6.1.4.1.38414.8.12.4
 fdVoicemailContexts            (FusionDirectory - available voicemail contexts)                - 1.3.6.1.4.1.38414.19.11.2
 ghScsiDev                      (Hardware definitions, value scsiDev)                           - 1.3.6.1.4.1.10098.1.1.2.5
 goFonAdmin                     (Admin user for fon server)                                     - 1.3.6.1.4.1.10098.1.1.9.26
 ghGfxAdapter                   (Hardware definitions, value Grafikkarte)                       - 1.3.6.1.4.1.10098.1.1.2.9
 fdPasswordHook                 (FusionDirectory - Password hook (external command))            - 1.3.6.1.4.1.38414.8.13.4
 gotoXKbModel                   (GOto - Gonicus Terminal Concept, value xKbmodel.)              - 1.3.6.1.4.1.10098.1.1.1.25
 gotoRootPasswd                 (GOto - Gonicus Terminal Concept, value rootPasswd.)            - 1.3.6.1.4.1.10098.1.1.1.14
 goLogDB                        (GOsa logging DB name)                                          - 1.3.6.1.4.1.10098.1.1.9.83
 goLogAdmin                     (Admin user for log server)                                     - 1.3.6.1.4.1.10098.1.1.9.24
 gotoXResolution                (GOto - Gonicus Terminal Concept, value xResolution.)           - 1.3.6.1.4.1.10098.1.1.1.20
 gotoScannerClients             (GOto - Gonicus Terminal Concept, value scannerClients.)        - 1.3.6.1.4.1.10098.1.1.1.11
 gotoFontPath                   (GOto - Gonicus Terminal Concept, value fontPath.)              - 1.3.6.1.4.1.10098.1.1.1.5
 goFonPassword                  (Admin password for fon server)                                 - 1.3.6.1.4.1.10098.1.1.9.27
Deprecated objectClasses:
 goNfsServer                    (NFS server description)                                        - 1.3.6.1.4.1.10098.1.2.1.19
 goCupsServer                   (CUPS server description)                                       - 1.3.6.1.4.1.10098.1.2.1.23
 gosaCacheEntry                 (GOsa - Class for GOsa caching)                                 - 1.3.6.1.4.1.10098.1.2.1.19.3
 gosaObject                     (GOsa - Class for GOsa settings)                                - 1.3.6.1.4.1.10098.1.2.1.19.1
 goFaxServer                    (Fax server description)                                        - 1.3.6.1.4.1.10098.1.2.1.26
 goFonServer                    (Fon server description)                                        - 1.3.6.1.4.1.10098.1.2.1.29
 gosaUserTemplate               (GOsa - Class for GOsa User Templates)                          - 1.3.6.1.4.1.10098.1.2.1.19.11
 gosaAccount                    (GOsa - Class for GOsa Accounts)                                - 1.3.6.1.4.1.10098.1.2.1.19.6
 goLogDBServer                  (Log DB server description)                                     - 1.3.6.1.4.1.10098.1.2.1.28
 goSyslogServer                 (Syslog server description)                                     - 1.3.6.1.4.1.10098.1.2.1.21
 goNtpServer                    (Time server description)                                       - 1.3.6.1.4.1.10098.1.2.1.20
```

fusiondirectory-setup --check-deprecated will output a list of dn using old attributes and objectClasses of they are present in your ldap server

```shell
fusiondirectory-setup --check-deprecated
List LDAP entries using deprecated attributes or objectclasses
There are no entries in the LDAP using obsolete attributes
There are no entries in the LDAP using obsolete classes
```

fusiondirectory-setup --ldif-deprecated will output an ldif file on the console that you can use with ldapmodify to clean you ldap server from old attributes.

If they are old objectClasses it will warn you and you will have to remove it by hand, they have been specified at the **fusiondirectory-setup --check-deprecated** step.

!! Please read it carefully before applying !!

### Migrate FusionDirectory from 1.0.12 to 1.0.13

### Ubuntu 12.0.4 TLS users
 
Since 1.0.9.2 FusionDirectory need the php-cas library for CAS server support. This library can normally found in universe in the Ubuntu repositories.

In case you did not find it, grab the deb from here [php cas][php-cas] and install it

### Upgrade FusionDirectory first

  * Upgrade FusionDirectory core package before other ones to avoid dependencies errors:

```shell
apt-get install fusiondirectory
```

  * Upgrade FusionDirectory schema package too.

```shell
apt-get install fusiondirectory-schema
```

### Upgrade of your LDAP directory

  * if your are using the alias plugin you have to update its schema

```shell
fusiondirectory-insert-schema -m /etc/ldap/schema/fusiondirectory/alias-fd.schema
```

### Remove old schema from /etc/ldap/schema/fusiondirectory

The old schema are not automatically removed from /etc/ldap/schema/fusiondirectory.
You can safely remove **recovery-fd.schema** and **asterisk-fd-conf.schema** if they still exist

### Check for deprecated attributes and objectClasses in your LDAP

fusiondirectory-setup --list-deprecated show deprecated attributes and objectClasses for FusionDirectory

```shell
fusiondirectory-setup --list-deprecated
List deprecated attributes and objectclasses
Deprecated attributes:
 gotoFilesystem                 (GOto - Gonicus Terminal Concept, value filesystem.)            - 1.3.6.1.4.1.10098.1.1.1.6
 gotoFloppyEnable               (GOto - Gonicus Terminal Concept, value floppyEnable.)          - 1.3.6.1.4.1.10098.1.1.1.7
 gotoSysStatus                  (Keeps current system status - info shown in GOsa)              - 1.3.6.1.4.1.10098.1.1.2.11
 gotoXColordepth                (GOto - Gonicus Terminal Concept, value xColordepth.)           - 1.3.6.1.4.1.10098.1.1.1.21
 gotoHardwareChecksum           (GOto - quick way to see if something has changed)              - 1.3.6.1.4.1.10098.1.1.2.12
 goFaxPassword                  (Admin password for fax server)                                 - 1.3.6.1.4.1.10098.1.1.9.23
 gotoNtpServer                  (GOto - Gonicus Terminal Concept, value ntpServer.)             - 1.3.6.1.4.1.10098.1.1.1.2
 gotoXDriver                    (GOto - Gonicus Terminal Concept, value xDriver.)               - 1.3.6.1.4.1.10098.1.1.1.28
 fdSnapshotAdminDn              (FusionDirectory - Snaphost admin dn)                           - 1.3.6.1.4.1.38414.8.17.4
 fdPersonalTitleInDN            (FusionDirectory - Personal title in dn)                        - 1.3.6.1.4.1.38414.8.12.5
 gotoXMouseport                 (GOto - Gonicus Terminal Concept, value xMouseport.)            - 1.3.6.1.4.1.10098.1.1.1.22
 gotoAdaptPath                  (GOto - Gonicus Terminal Concept, value adaptpath.)             - 1.3.6.1.4.1.10098.1.1.1.33
 goFonCountryCode               (Store country code)                                            - 1.3.6.1.4.1.10098.1.1.9.29
 gotoSndModule                  (GOto - Gonicus Terminal Concept, value sound Modules.)         - 1.3.6.1.4.1.10098.1.1.1.29
 gotoXKbLayout                  (GOto - Gonicus Terminal Concept, value xKblayout.)             - 1.3.6.1.4.1.10098.1.1.1.26
 gotoProfileFlags               (GOto - Flags for Profile handling - C is for caching)          - 1.3.6.1.4.1.10098.1.1.11.7
 ghSoundAdapter                 (Hardware definitions, value soundAdapter)                      - 1.3.6.1.4.1.10098.1.1.2.7
 gotoScannerModel               (GOto - Gonicus Terminal Concept, value scannerModel.)          - 1.3.6.1.4.1.10098.1.1.1.40
 gotoScannerEnable              (GOto - Gonicus Terminal Concept, value scannerEnable.)         - 1.3.6.1.4.1.10098.1.1.1.10
 gosaSubtreeACL                 (GOsa - ACL entry)                                              - 1.3.6.1.4.1.10098.1.1.12.1
 ghUsbSupport                   (Hardware definitions, value usbSupport)                        - 1.3.6.1.4.1.10098.1.1.2.3
 gotoLpdEnable                  (GOto - Gonicus Terminal Concept, value lpdEnable.)             - 1.3.6.1.4.1.10098.1.1.1.9
 gotoProfileQuota               (GOto - save quota for home)                                    - 1.3.6.1.4.1.10098.1.1.11.15
 gotoScannerBackend             (GOto - Gonicus Terminal Concept, value scannerBackend.)        - 1.3.6.1.4.1.10098.1.1.1.39
 goSyslogSection                (What sections wants the server for its syslog service? i.e. *.*)       - 1.3.6.1.4.1.10098.1.1.9.9
 fdAccountRDN                   (FusionDirectory - use a placeholder pattern for generating account RDNs)       - 1.3.6.1.4.1.38414.8.12.2
 goFaxAdmin                     (Admin principal for fax server)                                - 1.3.6.1.4.1.10098.1.1.9.22
 goLogDriver                    (FD logging MDB2 driver name)                                   - 1.3.6.1.4.1.10098.1.1.9.84
 ghNetNic                       (Hardware definitions, value Network Device)                    - 1.3.6.1.4.1.10098.1.1.2.8
 academicTitle                  (Field to represent the academic title)                         - 1.3.6.1.4.1.10098.1.1.6.2
 fdPhoneMacroRDN                (FusionDirectory - Phone macro RDN)                             - 1.3.6.1.4.1.38414.19.10.2
 gosaLoginRestriction           (GOsa - Multivalue attribute to carry a number of allowed ips/subnets)  - 1.3.6.1.4.1.10098.1.1.12.46
 gotoLpdServer                  (GOto - Gonicus Terminal Concept, value lpdServer.)             - 1.3.6.1.4.1.10098.1.1.1.4
 ghCpuType                      (Hardware definitions, value cpuType)                           - 1.3.6.1.4.1.10098.1.1.2.1
 gotoXMonitor                   (GOto - Gonicus Terminal Concept, value xMonitor.)              - 1.3.6.1.4.1.10098.1.1.1.17
 fdRfc2307bis                   (FusionDirectory - rfc2307bis)                                  - 1.3.6.1.4.1.38414.8.10.1
 gotoXVsync                     (GOto - Gonicus Terminal Concept, value xVsync.)                - 1.3.6.1.4.1.10098.1.1.1.19
 fdSipContexts                  (FusionDirectory - available sip contexts)                      - 1.3.6.1.4.1.38414.19.11.1
 gotoShare                      (GOto - specifies a share)                                      - 1.3.6.1.4.1.10098.1.1.11.9
 fdCopyPaste                    (FusionDirectory - (de)Activate copy/paste)                     - 1.3.6.1.4.1.38414.8.14.5
 goLogPassword                  (Admin password for log server)                                 - 1.3.6.1.4.1.10098.1.1.9.25
 gotoCdromEnable                (GOto - Gonicus Terminal Concept, value cdromEnable.)           - 1.3.6.1.4.1.10098.1.1.1.8
 gotoProfileServer              (GOto - specifies the profile server)                           - 1.3.6.1.4.1.10098.1.1.11.8
 fdSnapshotAdminPassword        (FusionDirectory - Snaphost admin password)                     - 1.3.6.1.4.1.38414.8.17.5
 ghMemSize                      (Hardware definitions, value memSize)                           - 1.3.6.1.4.1.10098.1.1.2.2
 fdSnapshotURI                  (FusionDirectory - Snaphost URI)                                - 1.3.6.1.4.1.38414.8.17.3
 gotoAutoFs                     (GOto - Gonicus Terminal Concept, value autofs.)                - 1.3.6.1.4.1.10098.1.1.1.31
 goFonAreaCode                  (Store area code)                                               - 1.3.6.1.4.1.10098.1.1.9.28
 ghInventoryNumber              (Unique number for inclusion in an inventory)                   - 1.3.6.1.4.1.10098.1.1.2.10
 fdPhoneConferenceRDN           (FusionDirectory - Phone conference RDN)                        - 1.3.6.1.4.1.38414.19.10.3
 gotoXMouseType                 (Hardware definitions, value Type of mouse)                     - 1.3.6.1.4.1.10098.1.1.1.34
 fdPrimaryGroupFilter           (FusionDirectory - Primary group filter)                        - 1.3.6.1.4.1.38414.8.14.1
 gotoXHsync                     (GOto - Gonicus Terminal Concept, value xHsync.)                - 1.3.6.1.4.1.10098.1.1.1.18
 gosaDefaultLanguage            (GOsa - Defines the default language for a user)                - 1.3.6.1.4.1.10098.1.1.12.14
 ghIdeDev                       (Hardware definitions, value ideDev)                            - 1.3.6.1.4.1.10098.1.1.2.4
 gotoXKbVariant                 (GOto - Gonicus Terminal Concept, value xKbvariant.)            - 1.3.6.1.4.1.10098.1.1.1.27
 gotoModules                    (GOto - Gonicus Terminal Concept, value kernel modules.)        - 1.3.6.1.4.1.10098.1.1.1.32
 gotoXMouseButtons              (GOto - Gonicus Terminal Concept, value xMouseButtons.)         - 1.3.6.1.4.1.10098.1.1.1.23
 fdIdGenerator                  (FusionDirectory - An automatic way to generate new user ids)   - 1.3.6.1.4.1.38414.8.12.4
 fdVoicemailContexts            (FusionDirectory - available voicemail contexts)                - 1.3.6.1.4.1.38414.19.11.2
 ghScsiDev                      (Hardware definitions, value scsiDev)                           - 1.3.6.1.4.1.10098.1.1.2.5
 goFonAdmin                     (Admin user for fon server)                                     - 1.3.6.1.4.1.10098.1.1.9.26
 ghGfxAdapter                   (Hardware definitions, value Grafikkarte)                       - 1.3.6.1.4.1.10098.1.1.2.9
 fdPasswordHook                 (FusionDirectory - Password hook (external command))            - 1.3.6.1.4.1.38414.8.13.4
 gotoXKbModel                   (GOto - Gonicus Terminal Concept, value xKbmodel.)              - 1.3.6.1.4.1.10098.1.1.1.25
 gotoRootPasswd                 (GOto - Gonicus Terminal Concept, value rootPasswd.)            - 1.3.6.1.4.1.10098.1.1.1.14
 goLogDB                        (GOsa logging DB name)                                          - 1.3.6.1.4.1.10098.1.1.9.83
 goLogAdmin                     (Admin user for log server)                                     - 1.3.6.1.4.1.10098.1.1.9.24
 gotoXResolution                (GOto - Gonicus Terminal Concept, value xResolution.)           - 1.3.6.1.4.1.10098.1.1.1.20
 gotoScannerClients             (GOto - Gonicus Terminal Concept, value scannerClients.)        - 1.3.6.1.4.1.10098.1.1.1.11
 gotoFontPath                   (GOto - Gonicus Terminal Concept, value fontPath.)              - 1.3.6.1.4.1.10098.1.1.1.5
 goFonPassword                  (Admin password for fon server)                                 - 1.3.6.1.4.1.10098.1.1.9.27
Deprecated objectClasses:
 goNfsServer                    (NFS server description)                                        - 1.3.6.1.4.1.10098.1.2.1.19
 goCupsServer                   (CUPS server description)                                       - 1.3.6.1.4.1.10098.1.2.1.23
 gosaCacheEntry                 (GOsa - Class for GOsa caching)                                 - 1.3.6.1.4.1.10098.1.2.1.19.3
 gosaObject                     (GOsa - Class for GOsa settings)                                - 1.3.6.1.4.1.10098.1.2.1.19.1
 goFaxServer                    (Fax server description)                                        - 1.3.6.1.4.1.10098.1.2.1.26
 goFonServer                    (Fon server description)                                        - 1.3.6.1.4.1.10098.1.2.1.29
 gosaUserTemplate               (GOsa - Class for GOsa User Templates)                          - 1.3.6.1.4.1.10098.1.2.1.19.11
 gosaAccount                    (GOsa - Class for GOsa Accounts)                                - 1.3.6.1.4.1.10098.1.2.1.19.6
 goLogDBServer                  (Log DB server description)                                     - 1.3.6.1.4.1.10098.1.2.1.28
 goSyslogServer                 (Syslog server description)                                     - 1.3.6.1.4.1.10098.1.2.1.21
 goNtpServer                    (Time server description)                                       - 1.3.6.1.4.1.10098.1.2.1.20
```

fusiondirectory-setup --check-deprecated will output a list of dn using old attributes and objectClasses of they are present in your ldap server

```
fusiondirectory-setup --check-deprecated
List LDAP entries using deprecated attributes or objectclasses
There are no entries in the LDAP using obsolete attributes
There are no entries in the LDAP using obsolete classes
```

fusiondirectory-setup --ldif-deprecated will output an ldif file on the console that you can use with ldapmodify to clean you ldap server from old attributes.

If they are old objectClasses it will warn you and you will have to remove it by hand, they have been specified at the **fusiondirectory-setup --check-deprecated** step.

!! Please read it carefully before applying !!


## Migrate FusionDirectory from 1.0.13 to 1.0.14

### Ubuntu 12.0.4 TLS users
 
Since 1.0.9.2 FusionDirectory need the php-cas library for CAS server support. This library can normally found in universe in the Ubuntu repositories.

In case you did not find it, grab the deb from here [php cas][php-cas] and install it

### Upgrade FusionDirectory first

  * Upgrade FusionDirectory core package before other ones to avoid dependencies errors:

```shell
apt-get install fusiondirectory
```

  * Upgrade FusionDirectory schema package too.

```shell
apt-get install fusiondirectory-schema
```

### Upgrade of your LDAP directory

  * if your are using the community plugin you have to update its schema 

fusiondirectory-insert-schema -m /etc/ldap/schema/fusiondirectory/community-fd.schema

  * if your are using the systems plugin you have to update its schema

fusiondirectory-insert-schema -m /etc/ldap/schema/fusiondirectory/systems-fd.schema

### Remove old schema from /etc/ldap/schema/fusiondirectory

The old schema are not automatically removed from /etc/ldap/schema/fusiondirectory.
You can safely remove **recovery-fd.schema** and **asterisk-fd-conf.schema** if they still exist

### Check for deprecated attributes and objectClasses in your LDAP

fusiondirectory-setup --list-deprecated show deprecated attributes and objectClasses for FusionDirectory

```shell
fusiondirectory-setup --list-deprecated
List deprecated attributes and objectclasses
Deprecated attributes:
 gotoFilesystem                 (GOto - Gonicus Terminal Concept, value filesystem.)            - 1.3.6.1.4.1.10098.1.1.1.6
 gotoFloppyEnable               (GOto - Gonicus Terminal Concept, value floppyEnable.)          - 1.3.6.1.4.1.10098.1.1.1.7
 gotoSysStatus                  (Keeps current system status - info shown in GOsa)              - 1.3.6.1.4.1.10098.1.1.2.11
 gotoXColordepth                (GOto - Gonicus Terminal Concept, value xColordepth.)           - 1.3.6.1.4.1.10098.1.1.1.21
 gotoHardwareChecksum           (GOto - quick way to see if something has changed)              - 1.3.6.1.4.1.10098.1.1.2.12
 goFaxPassword                  (Admin password for fax server)                                 - 1.3.6.1.4.1.10098.1.1.9.23
 gotoNtpServer                  (GOto - Gonicus Terminal Concept, value ntpServer.)             - 1.3.6.1.4.1.10098.1.1.1.2
 gotoXDriver                    (GOto - Gonicus Terminal Concept, value xDriver.)               - 1.3.6.1.4.1.10098.1.1.1.28
 fdSnapshotAdminDn              (FusionDirectory - Snaphost admin dn)                           - 1.3.6.1.4.1.38414.8.17.4
 fdPersonalTitleInDN            (FusionDirectory - Personal title in dn)                        - 1.3.6.1.4.1.38414.8.12.5
 gotoXMouseport                 (GOto - Gonicus Terminal Concept, value xMouseport.)            - 1.3.6.1.4.1.10098.1.1.1.22
 gotoAdaptPath                  (GOto - Gonicus Terminal Concept, value adaptpath.)             - 1.3.6.1.4.1.10098.1.1.1.33
 goFonCountryCode               (Store country code)                                            - 1.3.6.1.4.1.10098.1.1.9.29
 gotoSndModule                  (GOto - Gonicus Terminal Concept, value sound Modules.)         - 1.3.6.1.4.1.10098.1.1.1.29
 gotoXKbLayout                  (GOto - Gonicus Terminal Concept, value xKblayout.)             - 1.3.6.1.4.1.10098.1.1.1.26
 gotoProfileFlags               (GOto - Flags for Profile handling - C is for caching)          - 1.3.6.1.4.1.10098.1.1.11.7
 ghSoundAdapter                 (Hardware definitions, value soundAdapter)                      - 1.3.6.1.4.1.10098.1.1.2.7
 gotoScannerModel               (GOto - Gonicus Terminal Concept, value scannerModel.)          - 1.3.6.1.4.1.10098.1.1.1.40
 gotoScannerEnable              (GOto - Gonicus Terminal Concept, value scannerEnable.)         - 1.3.6.1.4.1.10098.1.1.1.10
 gosaSubtreeACL                 (GOsa - ACL entry)                                              - 1.3.6.1.4.1.10098.1.1.12.1
 ghUsbSupport                   (Hardware definitions, value usbSupport)                        - 1.3.6.1.4.1.10098.1.1.2.3
 gotoLpdEnable                  (GOto - Gonicus Terminal Concept, value lpdEnable.)             - 1.3.6.1.4.1.10098.1.1.1.9
 gotoProfileQuota               (GOto - save quota for home)                                    - 1.3.6.1.4.1.10098.1.1.11.15
 gotoScannerBackend             (GOto - Gonicus Terminal Concept, value scannerBackend.)        - 1.3.6.1.4.1.10098.1.1.1.39
 goSyslogSection                (What sections wants the server for its syslog service? i.e. *.*)       - 1.3.6.1.4.1.10098.1.1.9.9
 fdAccountRDN                   (FusionDirectory - use a placeholder pattern for generating account RDNs)       - 1.3.6.1.4.1.38414.8.12.2
 goFaxAdmin                     (Admin principal for fax server)                                - 1.3.6.1.4.1.10098.1.1.9.22
 goLogDriver                    (FD logging MDB2 driver name)                                   - 1.3.6.1.4.1.10098.1.1.9.84
 ghNetNic                       (Hardware definitions, value Network Device)                    - 1.3.6.1.4.1.10098.1.1.2.8
 academicTitle                  (Field to represent the academic title)                         - 1.3.6.1.4.1.10098.1.1.6.2
 fdPhoneMacroRDN                (FusionDirectory - Phone macro RDN)                             - 1.3.6.1.4.1.38414.19.10.2
 gosaLoginRestriction           (GOsa - Multivalue attribute to carry a number of allowed ips/subnets)  - 1.3.6.1.4.1.10098.1.1.12.46
 gotoLpdServer                  (GOto - Gonicus Terminal Concept, value lpdServer.)             - 1.3.6.1.4.1.10098.1.1.1.4
 ghCpuType                      (Hardware definitions, value cpuType)                           - 1.3.6.1.4.1.10098.1.1.2.1
 gotoXMonitor                   (GOto - Gonicus Terminal Concept, value xMonitor.)              - 1.3.6.1.4.1.10098.1.1.1.17
 fdRfc2307bis                   (FusionDirectory - rfc2307bis)                                  - 1.3.6.1.4.1.38414.8.10.1
 gotoXVsync                     (GOto - Gonicus Terminal Concept, value xVsync.)                - 1.3.6.1.4.1.10098.1.1.1.19
 fdSipContexts                  (FusionDirectory - available sip contexts)                      - 1.3.6.1.4.1.38414.19.11.1
 gotoShare                      (GOto - specifies a share)                                      - 1.3.6.1.4.1.10098.1.1.11.9
 fdCopyPaste                    (FusionDirectory - (de)Activate copy/paste)                     - 1.3.6.1.4.1.38414.8.14.5
 goLogPassword                  (Admin password for log server)                                 - 1.3.6.1.4.1.10098.1.1.9.25
 gotoCdromEnable                (GOto - Gonicus Terminal Concept, value cdromEnable.)           - 1.3.6.1.4.1.10098.1.1.1.8
 gotoProfileServer              (GOto - specifies the profile server)                           - 1.3.6.1.4.1.10098.1.1.11.8
 fdSnapshotAdminPassword        (FusionDirectory - Snaphost admin password)                     - 1.3.6.1.4.1.38414.8.17.5
 ghMemSize                      (Hardware definitions, value memSize)                           - 1.3.6.1.4.1.10098.1.1.2.2
 fdSnapshotURI                  (FusionDirectory - Snaphost URI)                                - 1.3.6.1.4.1.38414.8.17.3
 gotoAutoFs                     (GOto - Gonicus Terminal Concept, value autofs.)                - 1.3.6.1.4.1.10098.1.1.1.31
 goFonAreaCode                  (Store area code)                                               - 1.3.6.1.4.1.10098.1.1.9.28
 ghInventoryNumber              (Unique number for inclusion in an inventory)                   - 1.3.6.1.4.1.10098.1.1.2.10
 fdPhoneConferenceRDN           (FusionDirectory - Phone conference RDN)                        - 1.3.6.1.4.1.38414.19.10.3
 gotoXMouseType                 (Hardware definitions, value Type of mouse)                     - 1.3.6.1.4.1.10098.1.1.1.34
 fdPrimaryGroupFilter           (FusionDirectory - Primary group filter)                        - 1.3.6.1.4.1.38414.8.14.1
 gotoXHsync                     (GOto - Gonicus Terminal Concept, value xHsync.)                - 1.3.6.1.4.1.10098.1.1.1.18
 gosaDefaultLanguage            (GOsa - Defines the default language for a user)                - 1.3.6.1.4.1.10098.1.1.12.14
 ghIdeDev                       (Hardware definitions, value ideDev)                            - 1.3.6.1.4.1.10098.1.1.2.4
 gotoXKbVariant                 (GOto - Gonicus Terminal Concept, value xKbvariant.)            - 1.3.6.1.4.1.10098.1.1.1.27
 gotoModules                    (GOto - Gonicus Terminal Concept, value kernel modules.)        - 1.3.6.1.4.1.10098.1.1.1.32
 gotoXMouseButtons              (GOto - Gonicus Terminal Concept, value xMouseButtons.)         - 1.3.6.1.4.1.10098.1.1.1.23
 fdIdGenerator                  (FusionDirectory - An automatic way to generate new user ids)   - 1.3.6.1.4.1.38414.8.12.4
 fdVoicemailContexts            (FusionDirectory - available voicemail contexts)                - 1.3.6.1.4.1.38414.19.11.2
 ghScsiDev                      (Hardware definitions, value scsiDev)                           - 1.3.6.1.4.1.10098.1.1.2.5
 goFonAdmin                     (Admin user for fon server)                                     - 1.3.6.1.4.1.10098.1.1.9.26
 ghGfxAdapter                   (Hardware definitions, value Grafikkarte)                       - 1.3.6.1.4.1.10098.1.1.2.9
 fdPasswordHook                 (FusionDirectory - Password hook (external command))            - 1.3.6.1.4.1.38414.8.13.4
 gotoXKbModel                   (GOto - Gonicus Terminal Concept, value xKbmodel.)              - 1.3.6.1.4.1.10098.1.1.1.25
 gotoRootPasswd                 (GOto - Gonicus Terminal Concept, value rootPasswd.)            - 1.3.6.1.4.1.10098.1.1.1.14
 goLogDB                        (GOsa logging DB name)                                          - 1.3.6.1.4.1.10098.1.1.9.83
 goLogAdmin                     (Admin user for log server)                                     - 1.3.6.1.4.1.10098.1.1.9.24
 gotoXResolution                (GOto - Gonicus Terminal Concept, value xResolution.)           - 1.3.6.1.4.1.10098.1.1.1.20
 gotoScannerClients             (GOto - Gonicus Terminal Concept, value scannerClients.)        - 1.3.6.1.4.1.10098.1.1.1.11
 gotoFontPath                   (GOto - Gonicus Terminal Concept, value fontPath.)              - 1.3.6.1.4.1.10098.1.1.1.5
 goFonPassword                  (Admin password for fon server)                                 - 1.3.6.1.4.1.10098.1.1.9.27
Deprecated objectClasses:
 goNfsServer                    (NFS server description)                                        - 1.3.6.1.4.1.10098.1.2.1.19
 goCupsServer                   (CUPS server description)                                       - 1.3.6.1.4.1.10098.1.2.1.23
 gosaCacheEntry                 (GOsa - Class for GOsa caching)                                 - 1.3.6.1.4.1.10098.1.2.1.19.3
 gosaObject                     (GOsa - Class for GOsa settings)                                - 1.3.6.1.4.1.10098.1.2.1.19.1
 goFaxServer                    (Fax server description)                                        - 1.3.6.1.4.1.10098.1.2.1.26
 goFonServer                    (Fon server description)                                        - 1.3.6.1.4.1.10098.1.2.1.29
 gosaUserTemplate               (GOsa - Class for GOsa User Templates)                          - 1.3.6.1.4.1.10098.1.2.1.19.11
 gosaAccount                    (GOsa - Class for GOsa Accounts)                                - 1.3.6.1.4.1.10098.1.2.1.19.6
 goLogDBServer                  (Log DB server description)                                     - 1.3.6.1.4.1.10098.1.2.1.28
 goSyslogServer                 (Syslog server description)                                     - 1.3.6.1.4.1.10098.1.2.1.21
 goNtpServer                    (Time server description)                                       - 1.3.6.1.4.1.10098.1.2.1.20
```

fusiondirectory-setup --check-deprecated will output a list of dn using old attributes and objectClasses of they are present in your ldap server

```shell
fusiondirectory-setup --check-deprecated
List LDAP entries using deprecated attributes or objectclasses
There are no entries in the LDAP using obsolete attributes
There are no entries in the LDAP using obsolete classes
```

fusiondirectory-setup --ldif-deprecated will output an ldif file on the console that you can use with ldapmodify to clean you ldap server from old attributes.

If they are old objectClasses it will warn you and you will have to remove it by hand, they have been specified at the **fusiondirectory-setup --check-deprecated** step.

!! Please read it carefully before applying !!

### New Breezy Theme

In this version by default there is a new more modern theme called breezy. To active it completely go to the configuration plugin, click edit and in the "Look and feel" section select breezy


## Migrate FusionDirectory from 1.0.14 to 1.0.15

### Ubuntu 12.0.4 TLS users
 
Since 1.0.9.2 FusionDirectory need the php-cas library for CAS server support. This library can normally found in universe in the Ubuntu repositories.

In case you did not find it, grab the deb from here [php cas][php-cas] and install it

### Upgrade FusionDirectory first

  * Upgrade FusionDirectory core package before other ones to avoid dependencies errors:

```shell
apt-get install fusiondirectory
```

  * Upgrade FusionDirectory schema package too.

```shell
apt-get install fusiondirectory-schema
```

### Removed plugin

The apache2 plugin as been removed in this version

```shell
apt-get remove fusiondirectory-plugin-apache2

apt-get remove fusiondirectory-plugin-apache2-schema
```

### Upgrade of your LDAP directory

  * Upgrade the core template schema 

```shell
fusiondirectory-insert-schema -m /etc/ldap/schema/fusiondirectory/core-fd-conf.schema

fusiondirectory-insert-schema -m /etc/ldap/schema/fusiondirectory/core-fd.schema
```

  * if your are using the systems plugin you have to update its schema

```shell
fusiondirectory-insert-schema -m /etc/ldap/schema/fusiondirectory/systems-fd.schema
```

  * if your are using the personal plugin you have to update its schema

```shell
fusiondirectory-insert-schema -m /etc/ldap/schema/fusiondirectory/personal-fd.schema
```

### Remove old schema from /etc/ldap/schema/fusiondirectory

The old schema are not automatically removed from /etc/ldap/schema/fusiondirectory.
You can safely remove **recovery-fd.schema** and **asterisk-fd-conf.schema** if they still exist

### Check for deprecated attributes and objectClasses in your LDAP

fusiondirectory-setup --list-deprecated show deprecated attributes and objectClasses for FusionDirectory

```shell
fusiondirectory-setup --list-deprecated
List deprecated attributes and objectclasses
Deprecated attributes:
 gotoXKbVariant                	(GOto - Gonicus Terminal Concept, value xKbvariant.)        	- 1.3.6.1.4.1.10098.1.1.1.27
 ghGfxAdapter                  	(Hardware definitions, value Grafikkarte)                   	- 1.3.6.1.4.1.10098.1.1.2.9
 gotoModules                   	(GOto - Gonicus Terminal Concept, value kernel modules.)    	- 1.3.6.1.4.1.10098.1.1.1.32
 gotoHardwareChecksum          	(GOto - quick way to see if something has changed)          	- 1.3.6.1.4.1.10098.1.1.2.12
 gotoPrinterPPD                	(GOto - Gonicus Terminal Concept, PPD data)                 	- 1.3.6.1.4.1.10098.1.1.11.6
 fdAsteriskDriver              	(Driver used for asterisk DB)                               	- 1.3.6.1.4.1.10098.1.1.9.30
 fdRfc2307bis                  	(FusionDirectory - rfc2307bis)                              	- 1.3.6.1.4.1.38414.8.10.1
 goFonAreaCode                 	(Store area code)                                           	- 1.3.6.1.4.1.10098.1.1.9.28
 gotoGroupAdminPrinter         	(GOto - keeps printers we are admin for)                    	- 1.3.6.1.4.1.10098.1.1.11.17
 gotoXVsync                    	(GOto - Gonicus Terminal Concept, value xVsync.)            	- 1.3.6.1.4.1.10098.1.1.1.19
 printerWindowsDriverName      	(Windows name of the printer driver)                        	- 1.3.6.1.4.1.38414.6.10.3
 printerWindowsDriverDir       	(Path to directory that contains windows drivers for this printer)	- 1.3.6.1.4.1.38414.6.10.2
 fdSipContexts                 	(FusionDirectory - available sip contexts)                  	- 1.3.6.1.4.1.38414.19.11.1
 fdPhoneConferenceRDN          	(FusionDirectory - Phone conference RDN)                    	- 1.3.6.1.4.1.38414.19.10.3
 goFonPassword                 	(Admin password for fon server)                             	- 1.3.6.1.4.1.10098.1.1.9.27
 fdSnapshotURI                 	(FusionDirectory - Snaphost URI)                            	- 1.3.6.1.4.1.38414.8.17.3
 fdSnapshotAdminPassword       	(FusionDirectory - Snaphost admin password)                 	- 1.3.6.1.4.1.38414.8.17.5
 gotoAdaptPath                 	(GOto - Gonicus Terminal Concept, value adaptpath.)         	- 1.3.6.1.4.1.10098.1.1.1.33
 ghMemSize                     	(Hardware definitions, value memSize)                       	- 1.3.6.1.4.1.10098.1.1.2.2
 gotoCdromEnable               	(GOto - Gonicus Terminal Concept, value cdromEnable.)       	- 1.3.6.1.4.1.10098.1.1.1.8
 ghInventoryNumber             	(Unique number for inclusion in an inventory)               	- 1.3.6.1.4.1.10098.1.1.2.10
 avArchiveMaxRecursion         	(Maximum number of archive nestings)                        	- 1.3.6.1.4.1.10098.1.1.9.73
 goLogAdmin                    	(Admin user for log server)                                 	- 1.3.6.1.4.1.10098.1.1.9.24
 gotoXMouseport                	(GOto - Gonicus Terminal Concept, value xMouseport.)        	- 1.3.6.1.4.1.10098.1.1.1.22
 gotoXMouseButtons             	(GOto - Gonicus Terminal Concept, value xMouseButtons.)     	- 1.3.6.1.4.1.10098.1.1.1.23
 gotoXKbLayout                 	(GOto - Gonicus Terminal Concept, value xKblayout.)         	- 1.3.6.1.4.1.10098.1.1.1.26
 fdCopyPaste                   	(FusionDirectory - (de)Activate copy/paste)                 	- 1.3.6.1.4.1.38414.8.14.5
 gotoScannerBackend            	(GOto - Gonicus Terminal Concept, value scannerBackend.)    	- 1.3.6.1.4.1.10098.1.1.1.39
 printerWindowsInfFile         	(Path to windows inf file for this printer)                 	- 1.3.6.1.4.1.38414.6.10.1
 avArchiveMaxCompressionRatio  	(Maximum compression ratio)                                 	- 1.3.6.1.4.1.10098.1.1.9.74
 fdPhoneMacroRDN               	(FusionDirectory - Phone macro RDN)                         	- 1.3.6.1.4.1.38414.19.10.2
 avMaxDirectoryRecursions      	(Number of recursions done with directories)                	- 1.3.6.1.4.1.10098.1.1.9.69
 gotoUserPrinter               	(GOto - keeps printers shown for this user)                 	- 1.3.6.1.4.1.10098.1.1.11.12
 fdPasswordHook                	(FusionDirectory - Password hook (external command))        	- 1.3.6.1.4.1.38414.8.13.4
 gotoXColordepth               	(GOto - Gonicus Terminal Concept, value xColordepth.)       	- 1.3.6.1.4.1.10098.1.1.1.21
 gotoXHsync                    	(GOto - Gonicus Terminal Concept, value xHsync.)            	- 1.3.6.1.4.1.10098.1.1.1.18
 fdPersonalTitleInDN           	(FusionDirectory - Personal title in dn)                    	- 1.3.6.1.4.1.38414.8.12.5
 gotoXMouseType                	(Hardware definitions, value Type of mouse)                 	- 1.3.6.1.4.1.10098.1.1.1.34
 gotoNtpServer                 	(GOto - Gonicus Terminal Concept, value ntpServer.)         	- 1.3.6.1.4.1.10098.1.1.1.2
 goLogDriver                   	(FD logging MDB2 driver name)                               	- 1.3.6.1.4.1.10098.1.1.9.84
 fdPrimaryGroupFilter          	(FusionDirectory - Primary group filter)                    	- 1.3.6.1.4.1.38414.8.14.1
 ghUsbSupport                  	(Hardware definitions, value usbSupport)                    	- 1.3.6.1.4.1.10098.1.1.2.3
 gotoXKbModel                  	(GOto - Gonicus Terminal Concept, value xKbmodel.)          	- 1.3.6.1.4.1.10098.1.1.1.25
 gotoSysStatus                 	(Keeps current system status - info shown in GOsa)          	- 1.3.6.1.4.1.10098.1.1.2.11
 goLogDB                       	(GOsa logging DB name)                                      	- 1.3.6.1.4.1.10098.1.1.9.83
 gotoFloppyEnable              	(GOto - Gonicus Terminal Concept, value floppyEnable.)      	- 1.3.6.1.4.1.10098.1.1.1.7
 gotoFontPath                  	(GOto - Gonicus Terminal Concept, value fontPath.)          	- 1.3.6.1.4.1.10098.1.1.1.5
 gotoAutoFs                    	(GOto - Gonicus Terminal Concept, value autofs.)            	- 1.3.6.1.4.1.10098.1.1.1.31
 gotoXDriver                   	(GOto - Gonicus Terminal Concept, value xDriver.)           	- 1.3.6.1.4.1.10098.1.1.1.28
 gotoXResolution               	(GOto - Gonicus Terminal Concept, value xResolution.)       	- 1.3.6.1.4.1.10098.1.1.1.20
 goFonCountryCode              	(Store country code)                                        	- 1.3.6.1.4.1.10098.1.1.9.29
 fdAccountRDN                  	(FusionDirectory - use a placeholder pattern for generating account RDNs)	- 1.3.6.1.4.1.38414.8.12.2
 avMaxThreads                  	(Number of AV scanning threads)                             	- 1.3.6.1.4.1.10098.1.1.9.68
 gotoScannerModel              	(GOto - Gonicus Terminal Concept, value scannerModel.)      	- 1.3.6.1.4.1.10098.1.1.1.40
 gotoSndModule                 	(GOto - Gonicus Terminal Concept, value sound Modules.)     	- 1.3.6.1.4.1.10098.1.1.1.29
 gotoLpdEnable                 	(GOto - Gonicus Terminal Concept, value lpdEnable.)         	- 1.3.6.1.4.1.10098.1.1.1.9
 avHttpProxyURL                	(How to get the updates)                                    	- 1.3.6.1.4.1.10098.1.1.9.76
 gotoRootPasswd                	(GOto - Gonicus Terminal Concept, value rootPasswd.)        	- 1.3.6.1.4.1.10098.1.1.1.14
 goLogPassword                 	(Admin password for log server)                             	- 1.3.6.1.4.1.10098.1.1.9.25
 gotoProfileServer             	(GOto - specifies the profile server)                       	- 1.3.6.1.4.1.10098.1.1.11.8
 avDatabaseMirror              	(Where to find updates)                                     	- 1.3.6.1.4.1.10098.1.1.9.75
 gotoGroupPrinter              	(GOto - keeps printers shown for this user)                 	- 1.3.6.1.4.1.10098.1.1.11.16
 fdMailMethod                  	(FusionDirectory - Mail method)                             	- 1.3.6.1.4.1.38414.10.10.1
 gotoFilesystem                	(GOto - Gonicus Terminal Concept, value filesystem.)        	- 1.3.6.1.4.1.10098.1.1.1.6
 ghSoundAdapter                	(Hardware definitions, value soundAdapter)                  	- 1.3.6.1.4.1.10098.1.1.2.7
 avArchiveMaxFileSize          	(Maximum archive file size)                                 	- 1.3.6.1.4.1.10098.1.1.9.72
 avChecksPerDay                	(Update checks per day)                                     	- 1.3.6.1.4.1.10098.1.1.9.78
 gotoScannerEnable             	(GOto - Gonicus Terminal Concept, value scannerEnable.)     	- 1.3.6.1.4.1.10098.1.1.1.10
 ghScsiDev                     	(Hardware definitions, value scsiDev)                       	- 1.3.6.1.4.1.10098.1.1.2.5
 goFaxPassword                 	(Admin password for fax server)                             	- 1.3.6.1.4.1.10098.1.1.9.23
 goSyslogSection               	(What sections wants the server for its syslog service? i.e. *.*)	- 1.3.6.1.4.1.10098.1.1.9.9
 gotoLpdServer                 	(GOto - Gonicus Terminal Concept, value lpdServer.)         	- 1.3.6.1.4.1.10098.1.1.1.4
 avUser                        	(Username to run antivirus with)                            	- 1.3.6.1.4.1.10098.1.1.9.70
 gotoProfileQuota              	(GOto - save quota for home)                                	- 1.3.6.1.4.1.10098.1.1.11.15
 fdIdGenerator                 	(FusionDirectory - An automatic way to generate new user ids)	- 1.3.6.1.4.1.38414.8.12.4
 avFlags                       	(Special flags for the antivirus scan engine)               	- 1.3.6.1.4.1.10098.1.1.9.71
 gotoScannerClients            	(GOto - Gonicus Terminal Concept, value scannerClients.)    	- 1.3.6.1.4.1.10098.1.1.1.11
 gotoXMonitor                  	(GOto - Gonicus Terminal Concept, value xMonitor.)          	- 1.3.6.1.4.1.10098.1.1.1.17
 ghNetNic                      	(Hardware definitions, value Network Device)                	- 1.3.6.1.4.1.10098.1.1.2.8
 goFonAdmin                    	(Admin user for fon server)                                 	- 1.3.6.1.4.1.10098.1.1.9.26
 gotoShare                     	(GOto - specifies a share)                                  	- 1.3.6.1.4.1.10098.1.1.11.9
 ghIdeDev                      	(Hardware definitions, value ideDev)                        	- 1.3.6.1.4.1.10098.1.1.2.4
 fdSnapshotAdminDn             	(FusionDirectory - Snaphost admin dn)                       	- 1.3.6.1.4.1.38414.8.17.4
 goFaxAdmin                    	(Admin principal for fax server)                            	- 1.3.6.1.4.1.10098.1.1.9.22
 ghCpuType                     	(Hardware definitions, value cpuType)                       	- 1.3.6.1.4.1.10098.1.1.2.1
 fdVoicemailContexts           	(FusionDirectory - available voicemail contexts)            	- 1.3.6.1.4.1.38414.19.11.2
 gotoProfileFlags              	(GOto - Flags for Profile handling - C is for caching)      	- 1.3.6.1.4.1.10098.1.1.11.7
 gotoUserAdminPrinter          	(GOto - keeps printers we are admin for)                    	- 1.3.6.1.4.1.10098.1.1.11.13
Deprecated objectClasses:
 goCupsServer                  	(CUPS server description)                                   	- 1.3.6.1.4.1.10098.1.2.1.23
 goLogDBServer                 	(Log DB server description)                                 	- 1.3.6.1.4.1.10098.1.2.1.28
 goFaxServer                   	(Fax server description)                                    	- 1.3.6.1.4.1.10098.1.2.1.26
 goNtpServer                   	(Time server description)                                   	- 1.3.6.1.4.1.10098.1.2.1.20
 goSyslogServer                	(Syslog server description)                                 	- 1.3.6.1.4.1.10098.1.2.1.21
 goNfsServer                   	(NFS server description)                                    	- 1.3.6.1.4.1.10098.1.2.1.19
 goFonServer                   	(Fon server description)                                    	- 1.3.6.1.4.1.10098.1.2.1.29
 gosaUserTemplate              	(GOsa - Class for GOsa User Templates)                      	- 1.3.6.1.4.1.10098.1.2.1.19.11
 goVirusServer                 	(Virus server definition)                                   	- 1.3.6.1.4.1.10098.1.2.1.39
```

fusiondirectory-setup --check-deprecated will output a list of dn using old attributes and objectClasses of they are present in your ldap server

```shell
fusiondirectory-setup --check-deprecated
List LDAP entries using deprecated attributes or objectclasses
There are no entries in the LDAP using obsolete attributes
There are no entries in the LDAP using obsolete classes
```

If you see the obsolete object class gotoPrinter you must run the next command and answer yes

```shell
fusiondirectory-setup --migrate-printers
```

fusiondirectory-setup --ldif-deprecated will output an ldif file on the console that you can use with ldapmodify to clean you ldap server from old attributes.

If they are old objectClasses it will warn you and you will have to remove it by hand, they have been specified at the fusiondirectory-setup --check-deprecated step.

!! Please read it carefully before applying !!

### Hook

You must modify your hook so that they don't use quotes. The attributes are now automatically quotes and escape.

### New Breezy Theme

In this version by default there is a new more modern theme called breezy. To active it completely go to the configuration plugin, click edit and in the "Look and feel" section select breezy


## Migrate FusionDirectory from 1.0.15 to 1.0.16

### Ubuntu 12.0.4 TLS users
 
Since 1.0.9.2 FusionDirectory need the php-cas library for CAS server support. This library can normally found in universe in the Ubuntu repositories.

In case you did not find it, grab the deb from here [php cas][php-cas] and install it

### Upgrade FusionDirectory first

  * Upgrade FusionDirectory core package before other ones to avoid dependencies errors:

```shell
apt-get install fusiondirectory
```

  * Upgrade FusionDirectory schema package too.

```shell
apt-get install fusiondirectory-schema
```

### Upgrade of your LDAP directory

  * Upgrade the core template schema

```shell
fusiondirectory-insert-schema -m /etc/ldap/schema/fusiondirectory/core-fd-conf.schema

fusiondirectory-insert-schema -m /etc/ldap/schema/fusiondirectory/core-fd.schema
```

### Check for deprecated attributes and objectClasses in your LDAP

The --list-deprecated option of fusiondirectory-setup show deprecated attributes and objectClasses for FusionDirectory

```shell
fusiondirectory-setup  --list-deprecated 
List deprecated attributes and objectclasses
Deprecated attributes:
 gotoLpdServer                  (GOto - Gonicus Terminal Concept, value lpdServer.)             - 1.3.6.1.4.1.10098.1.1.1.4
 fdPhoneMacroRDN                (FusionDirectory - Phone macro RDN)                             - 1.3.6.1.4.1.38414.19.10.2
 gotoCdromEnable                (GOto - Gonicus Terminal Concept, value cdromEnable.)           - 1.3.6.1.4.1.10098.1.1.1.8
 gotoFontPath                   (GOto - Gonicus Terminal Concept, value fontPath.)              - 1.3.6.1.4.1.10098.1.1.1.5
 printerWindowsDriverDir        (Path to directory that contains windows drivers for this printer)      - 1.3.6.1.4.1.38414.6.10.2
 avHttpProxyURL                 (How to get the updates)                                        - 1.3.6.1.4.1.10098.1.1.9.76
 avMaxDirectoryRecursions       (Number of recursions done with directories)                    - 1.3.6.1.4.1.10098.1.1.9.69
 goFonPassword                  (Admin password for fon server)                                 - 1.3.6.1.4.1.10098.1.1.9.27
 gotoFilesystem                 (GOto - Gonicus Terminal Concept, value filesystem.)            - 1.3.6.1.4.1.10098.1.1.1.6
 gotoPrinterPPD                 (GOto - Gonicus Terminal Concept, PPD data)                     - 1.3.6.1.4.1.10098.1.1.11.6
 fdSnapshotAdminDn              (FusionDirectory - Snaphost admin dn)                           - 1.3.6.1.4.1.38414.8.17.4
 ghUsbSupport                   (Hardware definitions, value usbSupport)                        - 1.3.6.1.4.1.10098.1.1.2.3
 gotoScannerEnable              (GOto - Gonicus Terminal Concept, value scannerEnable.)         - 1.3.6.1.4.1.10098.1.1.1.10
 gotoNtpServer                  (GOto - Gonicus Terminal Concept, value ntpServer.)             - 1.3.6.1.4.1.10098.1.1.1.2
 goFaxPassword                  (Admin password for fax server)                                 - 1.3.6.1.4.1.10098.1.1.9.23
 gotoSysStatus                  (Keeps current system status - info shown in GOsa)              - 1.3.6.1.4.1.10098.1.1.2.11
 gotoUserAdminPrinter           (GOto - keeps printers we are admin for)                        - 1.3.6.1.4.1.10098.1.1.11.13
 ghIdeDev                       (Hardware definitions, value ideDev)                            - 1.3.6.1.4.1.10098.1.1.2.4
 gotoSndModule                  (GOto - Gonicus Terminal Concept, value sound Modules.)         - 1.3.6.1.4.1.10098.1.1.1.29
 goFaxAdmin                     (Admin principal for fax server)                                - 1.3.6.1.4.1.10098.1.1.9.22
 gotoAdaptPath                  (GOto - Gonicus Terminal Concept, value adaptpath.)             - 1.3.6.1.4.1.10098.1.1.1.33
 gotoScannerModel               (GOto - Gonicus Terminal Concept, value scannerModel.)          - 1.3.6.1.4.1.10098.1.1.1.40
 gotoXColordepth                (GOto - Gonicus Terminal Concept, value xColordepth.)           - 1.3.6.1.4.1.10098.1.1.1.21
 fdAsteriskDriver               (Driver used for asterisk DB)                                   - 1.3.6.1.4.1.10098.1.1.9.30
 fdSipContexts                  (FusionDirectory - available sip contexts)                      - 1.3.6.1.4.1.38414.19.11.1
 gosaObject                     (GOsa - DN of an object)                                        - 1.3.6.1.4.1.10098.1.1.12.3
 ghInventoryNumber              (Unique number for inclusion in an inventory)                   - 1.3.6.1.4.1.10098.1.1.2.10
 ghSoundAdapter                 (Hardware definitions, value soundAdapter)                      - 1.3.6.1.4.1.10098.1.1.2.7
 ghGfxAdapter                   (Hardware definitions, value Grafikkarte)                       - 1.3.6.1.4.1.10098.1.1.2.9
 gotoXDriver                    (GOto - Gonicus Terminal Concept, value xDriver.)               - 1.3.6.1.4.1.10098.1.1.1.28
 gotoShare                      (GOto - specifies a share)                                      - 1.3.6.1.4.1.10098.1.1.11.9
 fdCopyPaste                    (FusionDirectory - (de)Activate copy/paste)                     - 1.3.6.1.4.1.38414.8.14.5
 gotoAutoFs                     (GOto - Gonicus Terminal Concept, value autofs.)                - 1.3.6.1.4.1.10098.1.1.1.31
 gotoScannerBackend             (GOto - Gonicus Terminal Concept, value scannerBackend.)        - 1.3.6.1.4.1.10098.1.1.1.39
 gotoUserPrinter                (GOto - keeps printers shown for this user)                     - 1.3.6.1.4.1.10098.1.1.11.12
 printerWindowsInfFile          (Path to windows inf file for this printer)                     - 1.3.6.1.4.1.38414.6.10.1
 avChecksPerDay                 (Update checks per day)                                         - 1.3.6.1.4.1.10098.1.1.9.78
 gotoScannerClients             (GOto - Gonicus Terminal Concept, value scannerClients.)        - 1.3.6.1.4.1.10098.1.1.1.11
 gotoXKbLayout                  (GOto - Gonicus Terminal Concept, value xKblayout.)             - 1.3.6.1.4.1.10098.1.1.1.26
 fdSnapshotAdminPassword        (FusionDirectory - Snaphost admin password)                     - 1.3.6.1.4.1.38414.8.17.5
 fdPersonalTitleInDN            (FusionDirectory - Personal title in dn)                        - 1.3.6.1.4.1.38414.8.12.5
 gotoXResolution                (GOto - Gonicus Terminal Concept, value xResolution.)           - 1.3.6.1.4.1.10098.1.1.1.20
 fdPasswordHook                 (FusionDirectory - Password hook (external command))            - 1.3.6.1.4.1.38414.8.13.4
 fdVoicemailContexts            (FusionDirectory - available voicemail contexts)                - 1.3.6.1.4.1.38414.19.11.2
 fdIdGenerator                  (FusionDirectory - An automatic way to generate new user ids)   - 1.3.6.1.4.1.38414.8.12.4
 goLogPassword                  (Admin password for log server)                                 - 1.3.6.1.4.1.10098.1.1.9.25
 ghCpuType                      (Hardware definitions, value cpuType)                           - 1.3.6.1.4.1.10098.1.1.2.1
 gotoGroupAdminPrinter          (GOto - keeps printers we are admin for)                        - 1.3.6.1.4.1.10098.1.1.11.17
 fdAccountRDN                   (FusionDirectory - use a placeholder pattern for generating account RDNs)       - 1.3.6.1.4.1.38414.8.12.2
 gotoFloppyEnable               (GOto - Gonicus Terminal Concept, value floppyEnable.)          - 1.3.6.1.4.1.10098.1.1.1.7
 goFonAreaCode                  (Store area code)                                               - 1.3.6.1.4.1.10098.1.1.9.28
 avArchiveMaxRecursion          (Maximum number of archive nestings)                            - 1.3.6.1.4.1.10098.1.1.9.73
 gotoRootPasswd                 (GOto - Gonicus Terminal Concept, value rootPasswd.)            - 1.3.6.1.4.1.10098.1.1.1.14
 fdPrimaryGroupFilter           (FusionDirectory - Primary group filter)                        - 1.3.6.1.4.1.38414.8.14.1
 goFonCountryCode               (Store country code)                                            - 1.3.6.1.4.1.10098.1.1.9.29
 avArchiveMaxFileSize           (Maximum archive file size)                                     - 1.3.6.1.4.1.10098.1.1.9.72
 gotoProfileServer              (GOto - specifies the profile server)                           - 1.3.6.1.4.1.10098.1.1.11.8
 goLogAdmin                     (Admin user for log server)                                     - 1.3.6.1.4.1.10098.1.1.9.24
 fdPhoneConferenceRDN           (FusionDirectory - Phone conference RDN)                        - 1.3.6.1.4.1.38414.19.10.3
 goFonAdmin                     (Admin user for fon server)                                     - 1.3.6.1.4.1.10098.1.1.9.26
 goLogDriver                    (FD logging MDB2 driver name)                                   - 1.3.6.1.4.1.10098.1.1.9.84
 gotoLpdEnable                  (GOto - Gonicus Terminal Concept, value lpdEnable.)             - 1.3.6.1.4.1.10098.1.1.1.9
 gotoModules                    (GOto - Gonicus Terminal Concept, value kernel modules.)        - 1.3.6.1.4.1.10098.1.1.1.32
 gotoProfileQuota               (GOto - save quota for home)                                    - 1.3.6.1.4.1.10098.1.1.11.15
 avFlags                        (Special flags for the antivirus scan engine)                   - 1.3.6.1.4.1.10098.1.1.9.71
 avUser                         (Username to run antivirus with)                                - 1.3.6.1.4.1.10098.1.1.9.70
 ghMemSize                      (Hardware definitions, value memSize)                           - 1.3.6.1.4.1.10098.1.1.2.2
 gotoXMonitor                   (GOto - Gonicus Terminal Concept, value xMonitor.)              - 1.3.6.1.4.1.10098.1.1.1.17
 gotoGroupPrinter               (GOto - keeps printers shown for this user)                     - 1.3.6.1.4.1.10098.1.1.11.16
 fdRfc2307bis                   (FusionDirectory - rfc2307bis)                                  - 1.3.6.1.4.1.38414.8.10.1
 gotoProfileFlags               (GOto - Flags for Profile handling - C is for caching)          - 1.3.6.1.4.1.10098.1.1.11.7
 fdMailMethod                   (FusionDirectory - Mail method)                                 - 1.3.6.1.4.1.38414.10.10.1
 gotoXKbVariant                 (GOto - Gonicus Terminal Concept, value xKbvariant.)            - 1.3.6.1.4.1.10098.1.1.1.27
 gotoXMouseport                 (GOto - Gonicus Terminal Concept, value xMouseport.)            - 1.3.6.1.4.1.10098.1.1.1.22
 fdSnapshotURI                  (FusionDirectory - Snaphost URI)                                - 1.3.6.1.4.1.38414.8.17.3
 gotoHardwareChecksum           (GOto - quick way to see if something has changed)              - 1.3.6.1.4.1.10098.1.1.2.12
 printerWindowsDriverName       (Windows name of the printer driver)                            - 1.3.6.1.4.1.38414.6.10.3
 ghScsiDev                      (Hardware definitions, value scsiDev)                           - 1.3.6.1.4.1.10098.1.1.2.5
 gotoXMouseType                 (Hardware definitions, value Type of mouse)                     - 1.3.6.1.4.1.10098.1.1.1.34
 gotoXVsync                     (GOto - Gonicus Terminal Concept, value xVsync.)                - 1.3.6.1.4.1.10098.1.1.1.19
 goSyslogSection                (What sections wants the server for its syslog service? i.e. *.*)       - 1.3.6.1.4.1.10098.1.1.9.9
 avDatabaseMirror               (Where to find updates)                                         - 1.3.6.1.4.1.10098.1.1.9.75
 goLogDB                        (GOsa logging DB name)                                          - 1.3.6.1.4.1.10098.1.1.9.83
 gosaUser                       (GOsa - DN of a user)                                           - 1.3.6.1.4.1.10098.1.1.12.2
 gotoXHsync                     (GOto - Gonicus Terminal Concept, value xHsync.)                - 1.3.6.1.4.1.10098.1.1.1.18
 avMaxThreads                   (Number of AV scanning threads)                                 - 1.3.6.1.4.1.10098.1.1.9.68
 gotoXMouseButtons              (GOto - Gonicus Terminal Concept, value xMouseButtons.)         - 1.3.6.1.4.1.10098.1.1.1.23
 avArchiveMaxCompressionRatio   (Maximum compression ratio)                                     - 1.3.6.1.4.1.10098.1.1.9.74
 ghNetNic                       (Hardware definitions, value Network Device)                    - 1.3.6.1.4.1.10098.1.1.2.8
 gotoXKbModel                   (GOto - Gonicus Terminal Concept, value xKbmodel.)              - 1.3.6.1.4.1.10098.1.1.1.25
Deprecated objectClasses:
 goFaxServer                    (Fax server description)                                        - 1.3.6.1.4.1.10098.1.2.1.26
 goSyslogServer                 (Syslog server description)                                     - 1.3.6.1.4.1.10098.1.2.1.21
 goCupsServer                   (CUPS server description)                                       - 1.3.6.1.4.1.10098.1.2.1.23
 goNtpServer                    (Time server description)                                       - 1.3.6.1.4.1.10098.1.2.1.20
 gosaLockEntry                  (GOsa - Class for GOsa locking)                                 - 1.3.6.1.4.1.10098.1.2.1.19.2
 goVirusServer                  (Virus server definition)                                       - 1.3.6.1.4.1.10098.1.2.1.39
 gosaUserTemplate               (GOsa - Class for GOsa User Templates)                          - 1.3.6.1.4.1.10098.1.2.1.19.11
 goNfsServer                    (NFS server description)                                        - 1.3.6.1.4.1.10098.1.2.1.19
 goFonServer                    (Fon server description)                                        - 1.3.6.1.4.1.10098.1.2.1.29
 goLogDBServer                  (Log DB server description)                                     - 1.3.6.1.4.1.10098.1.2.1.28
```

The --check-deprecated option will output a list of dn using old attributes and objectClasses of they are present in your ldap server

```shell
fusiondirectory-setup --check-deprecated
List LDAP entries using deprecated attributes or objectclasses
There are no entries in the LDAP using obsolete attributes
There are no entries in the LDAP using obsolete classes
```

The lock attribute and objectClass has been Change so you need to run fusiondirectory-setup --delete-gosa-locks to remove them

```shell
fusiondirectory-setup --delete-gosa-locks
Delete lock tokens using old gosaLockEntry class
Deleted lock 'cn=df0c0960f4202f794a331f07f3fbcccd,ou=locks,ou=fusiondirectory,dc=org'
Deleted lock 'cn=157dc41f15328c6ae3f1a2542c0dd481,ou=locks,ou=fusiondirectory,dc=org'
```

The --ldif-deprecated option will output an ldif file on the console that you can use with ldapmodify to clean you ldap server from old attributes.

fusiondirectory-setup --ldif-deprecated > remove_deprecated.ldif

If they are old objectClasses it will warn you and you will have to remove them by hand, they have been specified at the --check-deprecated step.

!! Please read it carefully before applying !!


## Migrate FusionDirectory from 1.0.16 to 1.0.17

### Ubuntu 12.0.4 TLS users
 
Since 1.0.9.2 FusionDirectory need the php-cas library for CAS server support. This library can normally found in universe in the Ubuntu repositories.

In case you did not find it, grab the deb from here [php cas][php-cas] and install it

### Upgrade FusionDirectory first

  * Upgrade FusionDirectory core package before other ones to avoid dependencies errors:

```shell
apt-get install fusiondirectory
```

  * Upgrade FusionDirectory schema package too.

```shell
apt-get install fusiondirectory-schema
```

## Upgrade of your LDAP directory

  * Upgrade schemas

if your are using the fai plugin you have to update its schema

```shell
fusiondirectory-insert-schema -m /etc/ldap/schema/fusiondirectory/fai.schema
```

if your are using the dhcp plugin you have to install a new schema

```shell
fusiondirectory-insert-schema -m /etc/ldap/schema/dhcp-fd-conf.schema
```

### Remove old ldif from /etc/ldap/schema/fusiondirectory

We removed a default ldif from the ppolicy plugin, you can delete it
safely from **/etc/ldap/schema/fusiondirectory/ppolicydefault.ldif**


### Check for deprecated attributes and objectClasses in your LDAP

The --list-deprecated option of fusiondirectory-setup show deprecated attributes and objectClasses for FusionDirectory

```shell
fusiondirectory-setup  --list-deprecated 
List deprecated attributes and objectclasses
Deprecated attributes:
 gotoLpdServer                  (GOto - Gonicus Terminal Concept, value lpdServer.)             - 1.3.6.1.4.1.10098.1.1.1.4
 fdPhoneMacroRDN                (FusionDirectory - Phone macro RDN)                             - 1.3.6.1.4.1.38414.19.10.2
 gotoCdromEnable                (GOto - Gonicus Terminal Concept, value cdromEnable.)           - 1.3.6.1.4.1.10098.1.1.1.8
 gotoFontPath                   (GOto - Gonicus Terminal Concept, value fontPath.)              - 1.3.6.1.4.1.10098.1.1.1.5
 printerWindowsDriverDir        (Path to directory that contains windows drivers for this printer)      - 1.3.6.1.4.1.38414.6.10.2
 avHttpProxyURL                 (How to get the updates)                                        - 1.3.6.1.4.1.10098.1.1.9.76
 avMaxDirectoryRecursions       (Number of recursions done with directories)                    - 1.3.6.1.4.1.10098.1.1.9.69
 goFonPassword                  (Admin password for fon server)                                 - 1.3.6.1.4.1.10098.1.1.9.27
 gotoFilesystem                 (GOto - Gonicus Terminal Concept, value filesystem.)            - 1.3.6.1.4.1.10098.1.1.1.6
 gotoPrinterPPD                 (GOto - Gonicus Terminal Concept, PPD data)                     - 1.3.6.1.4.1.10098.1.1.11.6
 fdSnapshotAdminDn              (FusionDirectory - Snaphost admin dn)                           - 1.3.6.1.4.1.38414.8.17.4
 ghUsbSupport                   (Hardware definitions, value usbSupport)                        - 1.3.6.1.4.1.10098.1.1.2.3
 gotoScannerEnable              (GOto - Gonicus Terminal Concept, value scannerEnable.)         - 1.3.6.1.4.1.10098.1.1.1.10
 gotoNtpServer                  (GOto - Gonicus Terminal Concept, value ntpServer.)             - 1.3.6.1.4.1.10098.1.1.1.2
 goFaxPassword                  (Admin password for fax server)                                 - 1.3.6.1.4.1.10098.1.1.9.23
 gotoSysStatus                  (Keeps current system status - info shown in GOsa)              - 1.3.6.1.4.1.10098.1.1.2.11
 gotoUserAdminPrinter           (GOto - keeps printers we are admin for)                        - 1.3.6.1.4.1.10098.1.1.11.13
 ghIdeDev                       (Hardware definitions, value ideDev)                            - 1.3.6.1.4.1.10098.1.1.2.4
 gotoSndModule                  (GOto - Gonicus Terminal Concept, value sound Modules.)         - 1.3.6.1.4.1.10098.1.1.1.29
 goFaxAdmin                     (Admin principal for fax server)                                - 1.3.6.1.4.1.10098.1.1.9.22
 gotoAdaptPath                  (GOto - Gonicus Terminal Concept, value adaptpath.)             - 1.3.6.1.4.1.10098.1.1.1.33
 gotoScannerModel               (GOto - Gonicus Terminal Concept, value scannerModel.)          - 1.3.6.1.4.1.10098.1.1.1.40
 gotoXColordepth                (GOto - Gonicus Terminal Concept, value xColordepth.)           - 1.3.6.1.4.1.10098.1.1.1.21
 fdAsteriskDriver               (Driver used for asterisk DB)                                   - 1.3.6.1.4.1.10098.1.1.9.30
 fdSipContexts                  (FusionDirectory - available sip contexts)                      - 1.3.6.1.4.1.38414.19.11.1
 gosaObject                     (GOsa - DN of an object)                                        - 1.3.6.1.4.1.10098.1.1.12.3
 ghInventoryNumber              (Unique number for inclusion in an inventory)                   - 1.3.6.1.4.1.10098.1.1.2.10
 ghSoundAdapter                 (Hardware definitions, value soundAdapter)                      - 1.3.6.1.4.1.10098.1.1.2.7
 ghGfxAdapter                   (Hardware definitions, value Grafikkarte)                       - 1.3.6.1.4.1.10098.1.1.2.9
 gotoXDriver                    (GOto - Gonicus Terminal Concept, value xDriver.)               - 1.3.6.1.4.1.10098.1.1.1.28
 gotoShare                      (GOto - specifies a share)                                      - 1.3.6.1.4.1.10098.1.1.11.9
 fdCopyPaste                    (FusionDirectory - (de)Activate copy/paste)                     - 1.3.6.1.4.1.38414.8.14.5
 gotoAutoFs                     (GOto - Gonicus Terminal Concept, value autofs.)                - 1.3.6.1.4.1.10098.1.1.1.31
 gotoScannerBackend             (GOto - Gonicus Terminal Concept, value scannerBackend.)        - 1.3.6.1.4.1.10098.1.1.1.39
 gotoUserPrinter                (GOto - keeps printers shown for this user)                     - 1.3.6.1.4.1.10098.1.1.11.12
 printerWindowsInfFile          (Path to windows inf file for this printer)                     - 1.3.6.1.4.1.38414.6.10.1
 avChecksPerDay                 (Update checks per day)                                         - 1.3.6.1.4.1.10098.1.1.9.78
 gotoScannerClients             (GOto - Gonicus Terminal Concept, value scannerClients.)        - 1.3.6.1.4.1.10098.1.1.1.11
 gotoXKbLayout                  (GOto - Gonicus Terminal Concept, value xKblayout.)             - 1.3.6.1.4.1.10098.1.1.1.26
 fdSnapshotAdminPassword        (FusionDirectory - Snaphost admin password)                     - 1.3.6.1.4.1.38414.8.17.5
 fdPersonalTitleInDN            (FusionDirectory - Personal title in dn)                        - 1.3.6.1.4.1.38414.8.12.5
 gotoXResolution                (GOto - Gonicus Terminal Concept, value xResolution.)           - 1.3.6.1.4.1.10098.1.1.1.20
 fdPasswordHook                 (FusionDirectory - Password hook (external command))            - 1.3.6.1.4.1.38414.8.13.4
 fdVoicemailContexts            (FusionDirectory - available voicemail contexts)                - 1.3.6.1.4.1.38414.19.11.2
 fdIdGenerator                  (FusionDirectory - An automatic way to generate new user ids)   - 1.3.6.1.4.1.38414.8.12.4
 goLogPassword                  (Admin password for log server)                                 - 1.3.6.1.4.1.10098.1.1.9.25
 ghCpuType                      (Hardware definitions, value cpuType)                           - 1.3.6.1.4.1.10098.1.1.2.1
 gotoGroupAdminPrinter          (GOto - keeps printers we are admin for)                        - 1.3.6.1.4.1.10098.1.1.11.17
 fdAccountRDN                   (FusionDirectory - use a placeholder pattern for generating account RDNs)       - 1.3.6.1.4.1.38414.8.12.2
 gotoFloppyEnable               (GOto - Gonicus Terminal Concept, value floppyEnable.)          - 1.3.6.1.4.1.10098.1.1.1.7
 goFonAreaCode                  (Store area code)                                               - 1.3.6.1.4.1.10098.1.1.9.28
 avArchiveMaxRecursion          (Maximum number of archive nestings)                            - 1.3.6.1.4.1.10098.1.1.9.73
 gotoRootPasswd                 (GOto - Gonicus Terminal Concept, value rootPasswd.)            - 1.3.6.1.4.1.10098.1.1.1.14
 fdPrimaryGroupFilter           (FusionDirectory - Primary group filter)                        - 1.3.6.1.4.1.38414.8.14.1
 goFonCountryCode               (Store country code)                                            - 1.3.6.1.4.1.10098.1.1.9.29
 avArchiveMaxFileSize           (Maximum archive file size)                                     - 1.3.6.1.4.1.10098.1.1.9.72
 gotoProfileServer              (GOto - specifies the profile server)                           - 1.3.6.1.4.1.10098.1.1.11.8
 goLogAdmin                     (Admin user for log server)                                     - 1.3.6.1.4.1.10098.1.1.9.24
 fdPhoneConferenceRDN           (FusionDirectory - Phone conference RDN)                        - 1.3.6.1.4.1.38414.19.10.3
 goFonAdmin                     (Admin user for fon server)                                     - 1.3.6.1.4.1.10098.1.1.9.26
 goLogDriver                    (FD logging MDB2 driver name)                                   - 1.3.6.1.4.1.10098.1.1.9.84
 gotoLpdEnable                  (GOto - Gonicus Terminal Concept, value lpdEnable.)             - 1.3.6.1.4.1.10098.1.1.1.9
 gotoModules                    (GOto - Gonicus Terminal Concept, value kernel modules.)        - 1.3.6.1.4.1.10098.1.1.1.32
 gotoProfileQuota               (GOto - save quota for home)                                    - 1.3.6.1.4.1.10098.1.1.11.15
 avFlags                        (Special flags for the antivirus scan engine)                   - 1.3.6.1.4.1.10098.1.1.9.71
 avUser                         (Username to run antivirus with)                                - 1.3.6.1.4.1.10098.1.1.9.70
 ghMemSize                      (Hardware definitions, value memSize)                           - 1.3.6.1.4.1.10098.1.1.2.2
 gotoXMonitor                   (GOto - Gonicus Terminal Concept, value xMonitor.)              - 1.3.6.1.4.1.10098.1.1.1.17
 gotoGroupPrinter               (GOto - keeps printers shown for this user)                     - 1.3.6.1.4.1.10098.1.1.11.16
 fdRfc2307bis                   (FusionDirectory - rfc2307bis)                                  - 1.3.6.1.4.1.38414.8.10.1
 gotoProfileFlags               (GOto - Flags for Profile handling - C is for caching)          - 1.3.6.1.4.1.10098.1.1.11.7
 fdMailMethod                   (FusionDirectory - Mail method)                                 - 1.3.6.1.4.1.38414.10.10.1
 gotoXKbVariant                 (GOto - Gonicus Terminal Concept, value xKbvariant.)            - 1.3.6.1.4.1.10098.1.1.1.27
 gotoXMouseport                 (GOto - Gonicus Terminal Concept, value xMouseport.)            - 1.3.6.1.4.1.10098.1.1.1.22
 fdSnapshotURI                  (FusionDirectory - Snaphost URI)                                - 1.3.6.1.4.1.38414.8.17.3
 gotoHardwareChecksum           (GOto - quick way to see if something has changed)              - 1.3.6.1.4.1.10098.1.1.2.12
 printerWindowsDriverName       (Windows name of the printer driver)                            - 1.3.6.1.4.1.38414.6.10.3
 ghScsiDev                      (Hardware definitions, value scsiDev)                           - 1.3.6.1.4.1.10098.1.1.2.5
 gotoXMouseType                 (Hardware definitions, value Type of mouse)                     - 1.3.6.1.4.1.10098.1.1.1.34
 gotoXVsync                     (GOto - Gonicus Terminal Concept, value xVsync.)                - 1.3.6.1.4.1.10098.1.1.1.19
 goSyslogSection                (What sections wants the server for its syslog service? i.e. *.*)       - 1.3.6.1.4.1.10098.1.1.9.9
 avDatabaseMirror               (Where to find updates)                                         - 1.3.6.1.4.1.10098.1.1.9.75
 goLogDB                        (GOsa logging DB name)                                          - 1.3.6.1.4.1.10098.1.1.9.83
 gosaUser                       (GOsa - DN of a user)                                           - 1.3.6.1.4.1.10098.1.1.12.2
 gotoXHsync                     (GOto - Gonicus Terminal Concept, value xHsync.)                - 1.3.6.1.4.1.10098.1.1.1.18
 avMaxThreads                   (Number of AV scanning threads)                                 - 1.3.6.1.4.1.10098.1.1.9.68
 gotoXMouseButtons              (GOto - Gonicus Terminal Concept, value xMouseButtons.)         - 1.3.6.1.4.1.10098.1.1.1.23
 avArchiveMaxCompressionRatio   (Maximum compression ratio)                                     - 1.3.6.1.4.1.10098.1.1.9.74
 ghNetNic                       (Hardware definitions, value Network Device)                    - 1.3.6.1.4.1.10098.1.1.2.8
 gotoXKbModel                   (GOto - Gonicus Terminal Concept, value xKbmodel.)              - 1.3.6.1.4.1.10098.1.1.1.25
Deprecated objectClasses:
 goFaxServer                    (Fax server description)                                        - 1.3.6.1.4.1.10098.1.2.1.26
 goSyslogServer                 (Syslog server description)                                     - 1.3.6.1.4.1.10098.1.2.1.21
 goCupsServer                   (CUPS server description)                                       - 1.3.6.1.4.1.10098.1.2.1.23
 goNtpServer                    (Time server description)                                       - 1.3.6.1.4.1.10098.1.2.1.20
 gosaLockEntry                  (GOsa - Class for GOsa locking)                                 - 1.3.6.1.4.1.10098.1.2.1.19.2
 goVirusServer                  (Virus server definition)                                       - 1.3.6.1.4.1.10098.1.2.1.39
 gosaUserTemplate               (GOsa - Class for GOsa User Templates)                          - 1.3.6.1.4.1.10098.1.2.1.19.11
 goNfsServer                    (NFS server description)                                        - 1.3.6.1.4.1.10098.1.2.1.19
 goFonServer                    (Fon server description)                                        - 1.3.6.1.4.1.10098.1.2.1.29
 goLogDBServer                  (Log DB server description)                                     - 1.3.6.1.4.1.10098.1.2.1.28
```

The --check-deprecated option will output a list of dn using old attributes and objectClasses of they are present in your ldap server

```shell
fusiondirectory-setup --check-deprecated
List LDAP entries using deprecated attributes or objectclasses
There are no entries in the LDAP using obsolete attributes
There are no entries in the LDAP using obsolete classes
```

The --ldif-deprecated option will output an ldif file on the console that you can use with ldapmodify to clean you ldap server from old attributes.

fusiondirectory-setup --ldif-deprecated > remove_deprecated.ldif

If they are old objectClasses it will warn you and you will have to remove them by hand, they have been specified at the --check-deprecated step.

!! Please read it carefully before applying !!

### Migrate your DHCP entries

if you where using the DHCP plugin in 1.0.16, you need to migrate your entries to the dhcp branch, run the **fusiondirectory-setup --migrate-dhcp** for this

fusiondirectory-setup --migrate-dhcp


### Checking your indexed attributes

Check that all you index still match with valid attributes present in your ldap directory 


## Migrate FusionDirectory from 1.0.17 to 1.0.18

### Ubuntu 12.0.4 TLS users
 
Since 1.0.9.2 FusionDirectory need the php-cas library for CAS server support. This library can normally found in universe in the Ubuntu repositories.

In case you did not find it, grab the deb from here [php cas][php-cas] and install it

### Upgrade FusionDirectory first

  * Upgrade FusionDirectory core package before other ones to avoid dependencies errors:

```shell
apt-get install fusiondirectory
```

  * Upgrade FusionDirectory schema package too.

```shell
apt-get install fusiondirectory-schema
```

### Upgrade of your LDAP directory

  * Upgrade the core schemas

```shell
fusiondirectory-insert-schema -m /etc/ldap/schema/fusiondirectory/core-fd-conf.schema
```

  * if your are using the systems plugin you have to update its schema

```shell
fusiondirectory-insert-schema -m /etc/ldap/schema/fusiondirectory/service-fd.schema

fusiondirectory-insert-schema -m /etc/ldap/schema/fusiondirectory/system-fd.schema
```

### Check for deprecated attributes and objectClasses in your LDAP

The --list-deprecated option of fusiondirectory-setup show deprecated attributes and objectClasses for FusionDirectory

```shell
fusiondirectory-setup  --list-deprecated 
List deprecated attributes and objectclasses
Deprecated attributes:
 gotoLpdServer                  (GOto - Gonicus Terminal Concept, value lpdServer.)             - 1.3.6.1.4.1.10098.1.1.1.4
 fdPhoneMacroRDN                (FusionDirectory - Phone macro RDN)                             - 1.3.6.1.4.1.38414.19.10.2
 gotoCdromEnable                (GOto - Gonicus Terminal Concept, value cdromEnable.)           - 1.3.6.1.4.1.10098.1.1.1.8
 gotoFontPath                   (GOto - Gonicus Terminal Concept, value fontPath.)              - 1.3.6.1.4.1.10098.1.1.1.5
 printerWindowsDriverDir        (Path to directory that contains windows drivers for this printer)      - 1.3.6.1.4.1.38414.6.10.2
 avHttpProxyURL                 (How to get the updates)                                        - 1.3.6.1.4.1.10098.1.1.9.76
 avMaxDirectoryRecursions       (Number of recursions done with directories)                    - 1.3.6.1.4.1.10098.1.1.9.69
 goFonPassword                  (Admin password for fon server)                                 - 1.3.6.1.4.1.10098.1.1.9.27
 gotoFilesystem                 (GOto - Gonicus Terminal Concept, value filesystem.)            - 1.3.6.1.4.1.10098.1.1.1.6
 gotoPrinterPPD                 (GOto - Gonicus Terminal Concept, PPD data)                     - 1.3.6.1.4.1.10098.1.1.11.6
 fdSnapshotAdminDn              (FusionDirectory - Snaphost admin dn)                           - 1.3.6.1.4.1.38414.8.17.4
 ghUsbSupport                   (Hardware definitions, value usbSupport)                        - 1.3.6.1.4.1.10098.1.1.2.3
 gotoScannerEnable              (GOto - Gonicus Terminal Concept, value scannerEnable.)         - 1.3.6.1.4.1.10098.1.1.1.10
 gotoNtpServer                  (GOto - Gonicus Terminal Concept, value ntpServer.)             - 1.3.6.1.4.1.10098.1.1.1.2
 goFaxPassword                  (Admin password for fax server)                                 - 1.3.6.1.4.1.10098.1.1.9.23
 gotoSysStatus                  (Keeps current system status - info shown in GOsa)              - 1.3.6.1.4.1.10098.1.1.2.11
 gotoUserAdminPrinter           (GOto - keeps printers we are admin for)                        - 1.3.6.1.4.1.10098.1.1.11.13
 ghIdeDev                       (Hardware definitions, value ideDev)                            - 1.3.6.1.4.1.10098.1.1.2.4
 gotoSndModule                  (GOto - Gonicus Terminal Concept, value sound Modules.)         - 1.3.6.1.4.1.10098.1.1.1.29
 goFaxAdmin                     (Admin principal for fax server)                                - 1.3.6.1.4.1.10098.1.1.9.22
 gotoAdaptPath                  (GOto - Gonicus Terminal Concept, value adaptpath.)             - 1.3.6.1.4.1.10098.1.1.1.33
 gotoScannerModel               (GOto - Gonicus Terminal Concept, value scannerModel.)          - 1.3.6.1.4.1.10098.1.1.1.40
 gotoXColordepth                (GOto - Gonicus Terminal Concept, value xColordepth.)           - 1.3.6.1.4.1.10098.1.1.1.21
 fdAsteriskDriver               (Driver used for asterisk DB)                                   - 1.3.6.1.4.1.10098.1.1.9.30
 fdSipContexts                  (FusionDirectory - available sip contexts)                      - 1.3.6.1.4.1.38414.19.11.1
 gosaObject                     (GOsa - DN of an object)                                        - 1.3.6.1.4.1.10098.1.1.12.3
 ghInventoryNumber              (Unique number for inclusion in an inventory)                   - 1.3.6.1.4.1.10098.1.1.2.10
 ghSoundAdapter                 (Hardware definitions, value soundAdapter)                      - 1.3.6.1.4.1.10098.1.1.2.7
 ghGfxAdapter                   (Hardware definitions, value Grafikkarte)                       - 1.3.6.1.4.1.10098.1.1.2.9
 gotoXDriver                    (GOto - Gonicus Terminal Concept, value xDriver.)               - 1.3.6.1.4.1.10098.1.1.1.28
 gotoShare                      (GOto - specifies a share)                                      - 1.3.6.1.4.1.10098.1.1.11.9
 fdCopyPaste                    (FusionDirectory - (de)Activate copy/paste)                     - 1.3.6.1.4.1.38414.8.14.5
 gotoAutoFs                     (GOto - Gonicus Terminal Concept, value autofs.)                - 1.3.6.1.4.1.10098.1.1.1.31
 gotoScannerBackend             (GOto - Gonicus Terminal Concept, value scannerBackend.)        - 1.3.6.1.4.1.10098.1.1.1.39
 gotoUserPrinter                (GOto - keeps printers shown for this user)                     - 1.3.6.1.4.1.10098.1.1.11.12
 printerWindowsInfFile          (Path to windows inf file for this printer)                     - 1.3.6.1.4.1.38414.6.10.1
 avChecksPerDay                 (Update checks per day)                                         - 1.3.6.1.4.1.10098.1.1.9.78
 gotoScannerClients             (GOto - Gonicus Terminal Concept, value scannerClients.)        - 1.3.6.1.4.1.10098.1.1.1.11
 gotoXKbLayout                  (GOto - Gonicus Terminal Concept, value xKblayout.)             - 1.3.6.1.4.1.10098.1.1.1.26
 fdSnapshotAdminPassword        (FusionDirectory - Snaphost admin password)                     - 1.3.6.1.4.1.38414.8.17.5
 fdPersonalTitleInDN            (FusionDirectory - Personal title in dn)                        - 1.3.6.1.4.1.38414.8.12.5
 gotoXResolution                (GOto - Gonicus Terminal Concept, value xResolution.)           - 1.3.6.1.4.1.10098.1.1.1.20
 fdPasswordHook                 (FusionDirectory - Password hook (external command))            - 1.3.6.1.4.1.38414.8.13.4
 fdVoicemailContexts            (FusionDirectory - available voicemail contexts)                - 1.3.6.1.4.1.38414.19.11.2
 fdIdGenerator                  (FusionDirectory - An automatic way to generate new user ids)   - 1.3.6.1.4.1.38414.8.12.4
 goLogPassword                  (Admin password for log server)                                 - 1.3.6.1.4.1.10098.1.1.9.25
 ghCpuType                      (Hardware definitions, value cpuType)                           - 1.3.6.1.4.1.10098.1.1.2.1
 gotoGroupAdminPrinter          (GOto - keeps printers we are admin for)                        - 1.3.6.1.4.1.10098.1.1.11.17
 fdAccountRDN                   (FusionDirectory - use a placeholder pattern for generating account RDNs)       - 1.3.6.1.4.1.38414.8.12.2
 gotoFloppyEnable               (GOto - Gonicus Terminal Concept, value floppyEnable.)          - 1.3.6.1.4.1.10098.1.1.1.7
 goFonAreaCode                  (Store area code)                                               - 1.3.6.1.4.1.10098.1.1.9.28
 avArchiveMaxRecursion          (Maximum number of archive nestings)                            - 1.3.6.1.4.1.10098.1.1.9.73
 gotoRootPasswd                 (GOto - Gonicus Terminal Concept, value rootPasswd.)            - 1.3.6.1.4.1.10098.1.1.1.14
 fdPrimaryGroupFilter           (FusionDirectory - Primary group filter)                        - 1.3.6.1.4.1.38414.8.14.1
 goFonCountryCode               (Store country code)                                            - 1.3.6.1.4.1.10098.1.1.9.29
 avArchiveMaxFileSize           (Maximum archive file size)                                     - 1.3.6.1.4.1.10098.1.1.9.72
 gotoProfileServer              (GOto - specifies the profile server)                           - 1.3.6.1.4.1.10098.1.1.11.8
 goLogAdmin                     (Admin user for log server)                                     - 1.3.6.1.4.1.10098.1.1.9.24
 fdPhoneConferenceRDN           (FusionDirectory - Phone conference RDN)                        - 1.3.6.1.4.1.38414.19.10.3
 goFonAdmin                     (Admin user for fon server)                                     - 1.3.6.1.4.1.10098.1.1.9.26
 goLogDriver                    (FD logging MDB2 driver name)                                   - 1.3.6.1.4.1.10098.1.1.9.84
 gotoLpdEnable                  (GOto - Gonicus Terminal Concept, value lpdEnable.)             - 1.3.6.1.4.1.10098.1.1.1.9
 gotoModules                    (GOto - Gonicus Terminal Concept, value kernel modules.)        - 1.3.6.1.4.1.10098.1.1.1.32
 gotoProfileQuota               (GOto - save quota for home)                                    - 1.3.6.1.4.1.10098.1.1.11.15
 avFlags                        (Special flags for the antivirus scan engine)                   - 1.3.6.1.4.1.10098.1.1.9.71
 avUser                         (Username to run antivirus with)                                - 1.3.6.1.4.1.10098.1.1.9.70
 ghMemSize                      (Hardware definitions, value memSize)                           - 1.3.6.1.4.1.10098.1.1.2.2
 gotoXMonitor                   (GOto - Gonicus Terminal Concept, value xMonitor.)              - 1.3.6.1.4.1.10098.1.1.1.17
 gotoGroupPrinter               (GOto - keeps printers shown for this user)                     - 1.3.6.1.4.1.10098.1.1.11.16
 fdRfc2307bis                   (FusionDirectory - rfc2307bis)                                  - 1.3.6.1.4.1.38414.8.10.1
 gotoProfileFlags               (GOto - Flags for Profile handling - C is for caching)          - 1.3.6.1.4.1.10098.1.1.11.7
 fdMailMethod                   (FusionDirectory - Mail method)                                 - 1.3.6.1.4.1.38414.10.10.1
 gotoXKbVariant                 (GOto - Gonicus Terminal Concept, value xKbvariant.)            - 1.3.6.1.4.1.10098.1.1.1.27
 gotoXMouseport                 (GOto - Gonicus Terminal Concept, value xMouseport.)            - 1.3.6.1.4.1.10098.1.1.1.22
 fdSnapshotURI                  (FusionDirectory - Snaphost URI)                                - 1.3.6.1.4.1.38414.8.17.3
 gotoHardwareChecksum           (GOto - quick way to see if something has changed)              - 1.3.6.1.4.1.10098.1.1.2.12
 printerWindowsDriverName       (Windows name of the printer driver)                            - 1.3.6.1.4.1.38414.6.10.3
 ghScsiDev                      (Hardware definitions, value scsiDev)                           - 1.3.6.1.4.1.10098.1.1.2.5
 gotoXMouseType                 (Hardware definitions, value Type of mouse)                     - 1.3.6.1.4.1.10098.1.1.1.34
 gotoXVsync                     (GOto - Gonicus Terminal Concept, value xVsync.)                - 1.3.6.1.4.1.10098.1.1.1.19
 goSyslogSection                (What sections wants the server for its syslog service? i.e. *.*)       - 1.3.6.1.4.1.10098.1.1.9.9
 avDatabaseMirror               (Where to find updates)                                         - 1.3.6.1.4.1.10098.1.1.9.75
 goLogDB                        (GOsa logging DB name)                                          - 1.3.6.1.4.1.10098.1.1.9.83
 gosaUser                       (GOsa - DN of a user)                                           - 1.3.6.1.4.1.10098.1.1.12.2
 gotoXHsync                     (GOto - Gonicus Terminal Concept, value xHsync.)                - 1.3.6.1.4.1.10098.1.1.1.18
 avMaxThreads                   (Number of AV scanning threads)                                 - 1.3.6.1.4.1.10098.1.1.9.68
 gotoXMouseButtons              (GOto - Gonicus Terminal Concept, value xMouseButtons.)         - 1.3.6.1.4.1.10098.1.1.1.23
 avArchiveMaxCompressionRatio   (Maximum compression ratio)                                     - 1.3.6.1.4.1.10098.1.1.9.74
 ghNetNic                       (Hardware definitions, value Network Device)                    - 1.3.6.1.4.1.10098.1.1.2.8
 gotoXKbModel                   (GOto - Gonicus Terminal Concept, value xKbmodel.)              - 1.3.6.1.4.1.10098.1.1.1.25
Deprecated objectClasses:
 goFaxServer                    (Fax server description)                                        - 1.3.6.1.4.1.10098.1.2.1.26
 goSyslogServer                 (Syslog server description)                                     - 1.3.6.1.4.1.10098.1.2.1.21
 goCupsServer                   (CUPS server description)                                       - 1.3.6.1.4.1.10098.1.2.1.23
 goNtpServer                    (Time server description)                                       - 1.3.6.1.4.1.10098.1.2.1.20
 gosaLockEntry                  (GOsa - Class for GOsa locking)                                 - 1.3.6.1.4.1.10098.1.2.1.19.2
 goVirusServer                  (Virus server definition)                                       - 1.3.6.1.4.1.10098.1.2.1.39
 gosaUserTemplate               (GOsa - Class for GOsa User Templates)                          - 1.3.6.1.4.1.10098.1.2.1.19.11
 goNfsServer                    (NFS server description)                                        - 1.3.6.1.4.1.10098.1.2.1.19
 goFonServer                    (Fon server description)                                        - 1.3.6.1.4.1.10098.1.2.1.29
 goLogDBServer                  (Log DB server description)                                     - 1.3.6.1.4.1.10098.1.2.1.28
```

The --check-deprecated option will output a list of dn using old attributes and objectClasses of they are present in your ldap server

```shell
fusiondirectory-setup --check-deprecated
List LDAP entries using deprecated attributes or objectclasses
There are no entries in the LDAP using obsolete attributes
There are no entries in the LDAP using obsolete classes
```

The --ldif-deprecated option will output an ldif file on the console that you can use with ldapmodify to clean you ldap server from old attributes.

fusiondirectory-setup --ldif-deprecated > remove_deprecated.ldif

If they are old objectClasses it will warn you and you will have to remove them by hand, they have been specified at the --check-deprecated step.

!! Please read it carefully before applying !!

### Correcting acl editownpwd

!!!! to fix the error in the installed editownpwd, you need to go !!!!

  * Acl roles
  * Open the editownpwd acl
  * Save it 

Its very important you do it, to know why look at https://gitlab.fusiondirectory.org//issues/5276

### Checking your indexed attributes

Check that all you index still match with valid attributes present in your ldap directory 

## Migrate FusionDirectory from 1.0.18 to 1.0.19

### Ubuntu 12.0.4 TLS users
 
Since 1.0.9.2 FusionDirectory need the php-cas library for CAS server support. This library can normally found in universe in the Ubuntu repositories.

In case you did not find it, grab the deb from here [php cas][php-cas] and install it

### Upgrade FusionDirectory first

  * Upgrade FusionDirectory core package before other ones to avoid dependencies errors:

```shell
apt-get install fusiondirectory
```

  * Upgrade FusionDirectory schema package too.

```shell
apt-get install fusiondirectory-schema
```

### Upgrade of your LDAP directory

No upgrade


## Migrate FusionDirectory from 1.0.19 to 1.0.20

### Ubuntu 12.0.4 TLS users
 
Since 1.0.9.2 FusionDirectory need the php-cas library for CAS server support. This library can normally found in universe in the Ubuntu repositories.

In case you did not find it, grab the deb from here [php cas][php-cas] and install it

### Upgrade FusionDirectory first

  * Upgrade FusionDirectory core package before other ones to avoid dependencies errors:

```shell
apt-get install fusiondirectory
```

  * Upgrade FusionDirectory schema package too.

```shell
apt-get install fusiondirectory-schema
```

### Upgrade of your LDAP directory

  * Upgrade the core schemas

```shell
fusiondirectory-insert-schema -m /etc/ldap/schema/fusiondirectory/core-fd-conf.schema

fusiondirectory-insert-schema -m /etc/ldap/schema/fusiondirectory/template-fd.schema
```

  * if your are using the argonaut plugin you have to update its schema

```shell
fusiondirectory-insert-schema -m /etc/ldap/schema/fusiondirectory/argonaut-fd.schema
```

### Check for deprecated attributes and objectClasses in your LDAP

The --list-deprecated option of fusiondirectory-setup show deprecated attributes and objectClasses for FusionDirectory

```shell
fusiondirectory-setup  --list-deprecated 
List deprecated attributes and objectclasses
Deprecated attributes:
 gotoLpdServer                  (GOto - Gonicus Terminal Concept, value lpdServer.)             - 1.3.6.1.4.1.10098.1.1.1.4
 fdPhoneMacroRDN                (FusionDirectory - Phone macro RDN)                             - 1.3.6.1.4.1.38414.19.10.2
 gotoCdromEnable                (GOto - Gonicus Terminal Concept, value cdromEnable.)           - 1.3.6.1.4.1.10098.1.1.1.8
 gotoFontPath                   (GOto - Gonicus Terminal Concept, value fontPath.)              - 1.3.6.1.4.1.10098.1.1.1.5
 printerWindowsDriverDir        (Path to directory that contains windows drivers for this printer)      - 1.3.6.1.4.1.38414.6.10.2
 avHttpProxyURL                 (How to get the updates)                                        - 1.3.6.1.4.1.10098.1.1.9.76
 avMaxDirectoryRecursions       (Number of recursions done with directories)                    - 1.3.6.1.4.1.10098.1.1.9.69
 goFonPassword                  (Admin password for fon server)                                 - 1.3.6.1.4.1.10098.1.1.9.27
 gotoFilesystem                 (GOto - Gonicus Terminal Concept, value filesystem.)            - 1.3.6.1.4.1.10098.1.1.1.6
 gotoPrinterPPD                 (GOto - Gonicus Terminal Concept, PPD data)                     - 1.3.6.1.4.1.10098.1.1.11.6
 fdSnapshotAdminDn              (FusionDirectory - Snaphost admin dn)                           - 1.3.6.1.4.1.38414.8.17.4
 ghUsbSupport                   (Hardware definitions, value usbSupport)                        - 1.3.6.1.4.1.10098.1.1.2.3
 gotoScannerEnable              (GOto - Gonicus Terminal Concept, value scannerEnable.)         - 1.3.6.1.4.1.10098.1.1.1.10
 gotoNtpServer                  (GOto - Gonicus Terminal Concept, value ntpServer.)             - 1.3.6.1.4.1.10098.1.1.1.2
 goFaxPassword                  (Admin password for fax server)                                 - 1.3.6.1.4.1.10098.1.1.9.23
 gotoSysStatus                  (Keeps current system status - info shown in GOsa)              - 1.3.6.1.4.1.10098.1.1.2.11
 gotoUserAdminPrinter           (GOto - keeps printers we are admin for)                        - 1.3.6.1.4.1.10098.1.1.11.13
 ghIdeDev                       (Hardware definitions, value ideDev)                            - 1.3.6.1.4.1.10098.1.1.2.4
 gotoSndModule                  (GOto - Gonicus Terminal Concept, value sound Modules.)         - 1.3.6.1.4.1.10098.1.1.1.29
 goFaxAdmin                     (Admin principal for fax server)                                - 1.3.6.1.4.1.10098.1.1.9.22
 gotoAdaptPath                  (GOto - Gonicus Terminal Concept, value adaptpath.)             - 1.3.6.1.4.1.10098.1.1.1.33
 gotoScannerModel               (GOto - Gonicus Terminal Concept, value scannerModel.)          - 1.3.6.1.4.1.10098.1.1.1.40
 gotoXColordepth                (GOto - Gonicus Terminal Concept, value xColordepth.)           - 1.3.6.1.4.1.10098.1.1.1.21
 fdAsteriskDriver               (Driver used for asterisk DB)                                   - 1.3.6.1.4.1.10098.1.1.9.30
 fdSipContexts                  (FusionDirectory - available sip contexts)                      - 1.3.6.1.4.1.38414.19.11.1
 gosaObject                     (GOsa - DN of an object)                                        - 1.3.6.1.4.1.10098.1.1.12.3
 ghInventoryNumber              (Unique number for inclusion in an inventory)                   - 1.3.6.1.4.1.10098.1.1.2.10
 ghSoundAdapter                 (Hardware definitions, value soundAdapter)                      - 1.3.6.1.4.1.10098.1.1.2.7
 ghGfxAdapter                   (Hardware definitions, value Grafikkarte)                       - 1.3.6.1.4.1.10098.1.1.2.9
 gotoXDriver                    (GOto - Gonicus Terminal Concept, value xDriver.)               - 1.3.6.1.4.1.10098.1.1.1.28
 gotoShare                      (GOto - specifies a share)                                      - 1.3.6.1.4.1.10098.1.1.11.9
 fdCopyPaste                    (FusionDirectory - (de)Activate copy/paste)                     - 1.3.6.1.4.1.38414.8.14.5
 gotoAutoFs                     (GOto - Gonicus Terminal Concept, value autofs.)                - 1.3.6.1.4.1.10098.1.1.1.31
 gotoScannerBackend             (GOto - Gonicus Terminal Concept, value scannerBackend.)        - 1.3.6.1.4.1.10098.1.1.1.39
 gotoUserPrinter                (GOto - keeps printers shown for this user)                     - 1.3.6.1.4.1.10098.1.1.11.12
 printerWindowsInfFile          (Path to windows inf file for this printer)                     - 1.3.6.1.4.1.38414.6.10.1
 avChecksPerDay                 (Update checks per day)                                         - 1.3.6.1.4.1.10098.1.1.9.78
 gotoScannerClients             (GOto - Gonicus Terminal Concept, value scannerClients.)        - 1.3.6.1.4.1.10098.1.1.1.11
 gotoXKbLayout                  (GOto - Gonicus Terminal Concept, value xKblayout.)             - 1.3.6.1.4.1.10098.1.1.1.26
 fdSnapshotAdminPassword        (FusionDirectory - Snaphost admin password)                     - 1.3.6.1.4.1.38414.8.17.5
 fdPersonalTitleInDN            (FusionDirectory - Personal title in dn)                        - 1.3.6.1.4.1.38414.8.12.5
 gotoXResolution                (GOto - Gonicus Terminal Concept, value xResolution.)           - 1.3.6.1.4.1.10098.1.1.1.20
 fdPasswordHook                 (FusionDirectory - Password hook (external command))            - 1.3.6.1.4.1.38414.8.13.4
 fdVoicemailContexts            (FusionDirectory - available voicemail contexts)                - 1.3.6.1.4.1.38414.19.11.2
 fdIdGenerator                  (FusionDirectory - An automatic way to generate new user ids)   - 1.3.6.1.4.1.38414.8.12.4
 goLogPassword                  (Admin password for log server)                                 - 1.3.6.1.4.1.10098.1.1.9.25
 ghCpuType                      (Hardware definitions, value cpuType)                           - 1.3.6.1.4.1.10098.1.1.2.1
 gotoGroupAdminPrinter          (GOto - keeps printers we are admin for)                        - 1.3.6.1.4.1.10098.1.1.11.17
 fdAccountRDN                   (FusionDirectory - use a placeholder pattern for generating account RDNs)       - 1.3.6.1.4.1.38414.8.12.2
 gotoFloppyEnable               (GOto - Gonicus Terminal Concept, value floppyEnable.)          - 1.3.6.1.4.1.10098.1.1.1.7
 goFonAreaCode                  (Store area code)                                               - 1.3.6.1.4.1.10098.1.1.9.28
 avArchiveMaxRecursion          (Maximum number of archive nestings)                            - 1.3.6.1.4.1.10098.1.1.9.73
 gotoRootPasswd                 (GOto - Gonicus Terminal Concept, value rootPasswd.)            - 1.3.6.1.4.1.10098.1.1.1.14
 fdPrimaryGroupFilter           (FusionDirectory - Primary group filter)                        - 1.3.6.1.4.1.38414.8.14.1
 goFonCountryCode               (Store country code)                                            - 1.3.6.1.4.1.10098.1.1.9.29
 avArchiveMaxFileSize           (Maximum archive file size)                                     - 1.3.6.1.4.1.10098.1.1.9.72
 gotoProfileServer              (GOto - specifies the profile server)                           - 1.3.6.1.4.1.10098.1.1.11.8
 goLogAdmin                     (Admin user for log server)                                     - 1.3.6.1.4.1.10098.1.1.9.24
 fdPhoneConferenceRDN           (FusionDirectory - Phone conference RDN)                        - 1.3.6.1.4.1.38414.19.10.3
 goFonAdmin                     (Admin user for fon server)                                     - 1.3.6.1.4.1.10098.1.1.9.26
 goLogDriver                    (FD logging MDB2 driver name)                                   - 1.3.6.1.4.1.10098.1.1.9.84
 gotoLpdEnable                  (GOto - Gonicus Terminal Concept, value lpdEnable.)             - 1.3.6.1.4.1.10098.1.1.1.9
 gotoModules                    (GOto - Gonicus Terminal Concept, value kernel modules.)        - 1.3.6.1.4.1.10098.1.1.1.32
 gotoProfileQuota               (GOto - save quota for home)                                    - 1.3.6.1.4.1.10098.1.1.11.15
 avFlags                        (Special flags for the antivirus scan engine)                   - 1.3.6.1.4.1.10098.1.1.9.71
 avUser                         (Username to run antivirus with)                                - 1.3.6.1.4.1.10098.1.1.9.70
 ghMemSize                      (Hardware definitions, value memSize)                           - 1.3.6.1.4.1.10098.1.1.2.2
 gotoXMonitor                   (GOto - Gonicus Terminal Concept, value xMonitor.)              - 1.3.6.1.4.1.10098.1.1.1.17
 gotoGroupPrinter               (GOto - keeps printers shown for this user)                     - 1.3.6.1.4.1.10098.1.1.11.16
 fdRfc2307bis                   (FusionDirectory - rfc2307bis)                                  - 1.3.6.1.4.1.38414.8.10.1
 gotoProfileFlags               (GOto - Flags for Profile handling - C is for caching)          - 1.3.6.1.4.1.10098.1.1.11.7
 fdMailMethod                   (FusionDirectory - Mail method)                                 - 1.3.6.1.4.1.38414.10.10.1
 gotoXKbVariant                 (GOto - Gonicus Terminal Concept, value xKbvariant.)            - 1.3.6.1.4.1.10098.1.1.1.27
 gotoXMouseport                 (GOto - Gonicus Terminal Concept, value xMouseport.)            - 1.3.6.1.4.1.10098.1.1.1.22
 fdSnapshotURI                  (FusionDirectory - Snaphost URI)                                - 1.3.6.1.4.1.38414.8.17.3
 gotoHardwareChecksum           (GOto - quick way to see if something has changed)              - 1.3.6.1.4.1.10098.1.1.2.12
 printerWindowsDriverName       (Windows name of the printer driver)                            - 1.3.6.1.4.1.38414.6.10.3
 ghScsiDev                      (Hardware definitions, value scsiDev)                           - 1.3.6.1.4.1.10098.1.1.2.5
 gotoXMouseType                 (Hardware definitions, value Type of mouse)                     - 1.3.6.1.4.1.10098.1.1.1.34
 gotoXVsync                     (GOto - Gonicus Terminal Concept, value xVsync.)                - 1.3.6.1.4.1.10098.1.1.1.19
 goSyslogSection                (What sections wants the server for its syslog service? i.e. *.*)       - 1.3.6.1.4.1.10098.1.1.9.9
 avDatabaseMirror               (Where to find updates)                                         - 1.3.6.1.4.1.10098.1.1.9.75
 goLogDB                        (GOsa logging DB name)                                          - 1.3.6.1.4.1.10098.1.1.9.83
 gosaUser                       (GOsa - DN of a user)                                           - 1.3.6.1.4.1.10098.1.1.12.2
 gotoXHsync                     (GOto - Gonicus Terminal Concept, value xHsync.)                - 1.3.6.1.4.1.10098.1.1.1.18
 avMaxThreads                   (Number of AV scanning threads)                                 - 1.3.6.1.4.1.10098.1.1.9.68
 gotoXMouseButtons              (GOto - Gonicus Terminal Concept, value xMouseButtons.)         - 1.3.6.1.4.1.10098.1.1.1.23
 avArchiveMaxCompressionRatio   (Maximum compression ratio)                                     - 1.3.6.1.4.1.10098.1.1.9.74
 ghNetNic                       (Hardware definitions, value Network Device)                    - 1.3.6.1.4.1.10098.1.1.2.8
 gotoXKbModel                   (GOto - Gonicus Terminal Concept, value xKbmodel.)              - 1.3.6.1.4.1.10098.1.1.1.25
Deprecated objectClasses:
 goFaxServer                    (Fax server description)                                        - 1.3.6.1.4.1.10098.1.2.1.26
 goSyslogServer                 (Syslog server description)                                     - 1.3.6.1.4.1.10098.1.2.1.21
 goCupsServer                   (CUPS server description)                                       - 1.3.6.1.4.1.10098.1.2.1.23
 goNtpServer                    (Time server description)                                       - 1.3.6.1.4.1.10098.1.2.1.20
 gosaLockEntry                  (GOsa - Class for GOsa locking)                                 - 1.3.6.1.4.1.10098.1.2.1.19.2
 goVirusServer                  (Virus server definition)                                       - 1.3.6.1.4.1.10098.1.2.1.39
 gosaUserTemplate               (GOsa - Class for GOsa User Templates)                          - 1.3.6.1.4.1.10098.1.2.1.19.11
 goNfsServer                    (NFS server description)                                        - 1.3.6.1.4.1.10098.1.2.1.19
 goFonServer                    (Fon server description)                                        - 1.3.6.1.4.1.10098.1.2.1.29
 goLogDBServer                  (Log DB server description)                                     - 1.3.6.1.4.1.10098.1.2.1.28
```

The --check-deprecated option will output a list of dn using old attributes and objectClasses of they are present in your ldap server

```shell
fusiondirectory-setup --check-deprecated
List LDAP entries using deprecated attributes or objectclasses
There are no entries in the LDAP using obsolete attributes
There are no entries in the LDAP using obsolete classes
```

The --ldif-deprecated option will output an ldif file on the console that you can use with ldapmodify to clean you ldap server from old attributes.

fusiondirectory-setup --ldif-deprecated > remove_deprecated.ldif

If they are old objectClasses it will warn you and you will have to remove them by hand, they have been specified at the --check-deprecated step.

!! Please read it carefully before applying !!

### Correcting acl editownpwd

!!!! to fix the error in the installed editownpwd, you need to go !!!!

  * Acl roles
  * Open the editownpwd acl
  * Save it 

Its very important you do it, to know why look at https://gitlab.fusiondirectory.org//issues/5276

### Checking your indexed attributes

Check that all you index still match with valid attributes present in your ldap directory 


## Migrate FusionDirectory from 1.0.20 to 1.1

### Ubuntu 12.0.4 TLS users
 
Since 1.0.9.2 FusionDirectory need the php-cas library for CAS server support. This library can normally found in universe in the Ubuntu repositories.

In case you did not find it, grab the deb from here [php cas][php-cas] and install it

### Removed plugins

  * If you used kolab2 plugin you need to remove the plugin and its schema with the following command

```shell
apt-get remove fusiondirectory-plugin-kolab2
apt-get remove fusiondirectory-plugin-kolab2-schema
```

### Upgrade schema2ldif


For Jessie distribution add extra repository

```shell
# fusiondirectory debian-extra repository
deb http://repos.fusiondirectory.org/fusiondirectory-extra/debian-jessie/ jessie main
```

  * Update the package list and upgrade schema2ldif

```shell
apt-get update
apt-get install schema2ldif
```

### Upgrade FusionDirectory first

  * Upgrade FusionDirectory core package before other ones to avoid dependencies errors:

```shell
apt-get install fusiondirectory
```

  * Upgrade FusionDirectory schema package too.

```shell
apt-get install fusiondirectory-schema
```
### Upgrade of LDAP directory

  * Upgrade the core schemas 

```shell
fusiondirectory-insert-schema -m /etc/ldap/schema/fusiondirectory/core-fd-conf.schema
fusiondirectory-insert-schema -m /etc/ldap/schema/fusiondirectory/core-fd.schema
```

  * if you are using the argonaut plugin you have to update its schema

```shell
fusiondirectory-insert-schema -m /etc/ldap/schema/fusiondirectory/argonaut-fd.schema
```

  * if you are using the dhcp plugin you have to update its schema

```shell
fusiondirectory-insert-schema -m /etc/ldap/schema/fusiondirectory/dhcp-fd.schema
```

  * if you are using the fai plugin you have to update its schema

```shell
fusiondirectory-insert-schema -m /etc/ldap/schema/fusiondirectory/fai.schema
```

  * if you are using the mail plugin you have to update its schema

```shell
fusiondirectory-insert-schema -m /etc/ldap/schema/fusiondirectory/mail-fd.schema
```

* if you are using the systems plugin you have to update its schema

```shell
fusiondirectory-insert-schema -m /etc/ldap/schema/fusiondirectory/service-fd.schema
fusiondirectory-insert-schema -m /etc/ldap/schema/fusiondirectory/systems-fd-conf.schema
fusiondirectory-insert-schema -m /etc/ldap/schema/fusiondirectory/systems-fd.schema
```

### Migrate old objects

  * If you have systems use this command to migrate them

```shell
fusiondirectory-setup --migrate-systems
```

  * If you have phones use this command to migrate them

```shell
fusiondirectory-setup --migrate-phones
```

  * If you have winstations use this command to migrate them

```shell
fusiondirectory-setup --migrate-winstations
```

### Check for deprecated attributes and objectClasses in your LDAP

The list-deprecated option of fusiondirectory-setup show deprecated attributes and objectClasses for FusionDirectory

```shell
fusiondirectory-setup  --list-deprecated                                            
List deprecated attributes and objectclasses
Deprecated attributes:
 fdRfc2307bis                  	(FusionDirectory - rfc2307bis)                              	- 1.3.6.1.4.1.38414.8.10.1
 gosaDefaultPrinter            	(Defines a default printer a user owns)                     	- 1.3.6.1.4.1.10098.1.1.12.13
 gosaUser                      	(GOsa - DN of a user)                                       	- 1.3.6.1.4.1.10098.1.1.12.2
 gotoLogoffScript              	(GOto - specifies a LogoffScript)                           	- 1.3.6.1.4.1.10098.1.1.11.19
 gotoSyslogServer              	(GOto - Gonicus Terminal Concept, value syslogServer.)      	- 1.3.6.1.4.1.10098.1.1.1.1
 gotoHotplugDeviceDN           	(GOto - points to hotplug devices)                          	- 1.3.6.1.4.1.10098.1.1.11.18
 argonautLdap2zoneAllowNotify  	(Fusion Directory - Argonaut, allow notify.)                	- 1.3.6.1.4.1.38414.2.13.2
 fdAccountRDN                  	(FusionDirectory - use a placeholder pattern for generating account RDNs)	- 1.3.6.1.4.1.38414.8.12.2
 fdMailMethod                  	(FusionDirectory - Mail method)                             	- 1.3.6.1.4.1.38414.10.10.1
 fdPrimaryGroupFilter          	(FusionDirectory - Primary group filter)                    	- 1.3.6.1.4.1.38414.8.14.1
 fdPasswordHook                	(FusionDirectory - Password hook (external command))        	- 1.3.6.1.4.1.38414.8.13.4
 fdCopyPaste                   	(FusionDirectory - (de)Activate copy/paste)                 	- 1.3.6.1.4.1.38414.8.14.5
 fdIdGenerator                 	(FusionDirectory - An automatic way to generate new user ids)	- 1.3.6.1.4.1.38414.8.12.4
 fdPersonalTitleInDN           	(FusionDirectory - Personal title in dn)                    	- 1.3.6.1.4.1.38414.8.12.5
 fdSnapshotAdminPassword       	(FusionDirectory - Snaphost admin password)                 	- 1.3.6.1.4.1.38414.8.17.5
 gotoLogonScript               	(GOto - specifies a LogonScript)                            	- 1.3.6.1.4.1.10098.1.1.11.10
 gotoMode                      	(GOto - Gonicus Terminal Concept, Terminal is active.)      	- 1.3.6.1.4.1.10098.1.1.1.24
 fdSnapshotURI                 	(FusionDirectory - Snaphost URI)                            	- 1.3.6.1.4.1.38414.8.17.3
 fdSnapshotAdminDn             	(FusionDirectory - Snaphost admin dn)                       	- 1.3.6.1.4.1.38414.8.17.4
 gotoHotplugDevice             	(GOto - keeps hotplug devices)                              	- 1.3.6.1.4.1.10098.1.1.11.14
 gosaObject                    	(GOsa - DN of an object)                                    	- 1.3.6.1.4.1.10098.1.1.12.3
 gotoLdapServer                	(LDAP server to use)                                        	- 1.3.6.1.4.1.10098.1.1.1.38
Deprecated objectClasses:
 gotoTerminalTemplate          	(GOto - Gonicus Terminal Concept, objectclass)              	- 1.3.6.1.4.1.10098.1.2.1.35
 gotoEnvironment               	(GOto - contains environment settings)                      	- 1.3.6.1.4.1.10098.1.2.1.32
 gosaLockEntry                 	(GOsa - Class for GOsa locking)                             	- 1.3.6.1.4.1.10098.1.2.1.19.2
 goServer                      	(Server description)                                        	- 1.3.6.1.4.1.10098.1.2.1.27
 GOhard                        	(Gonicus Hardware definitions, objectclass)                 	- 1.3.6.1.4.1.10098.1.2.1.3
 fdAsteriskPluginConf          	(FusionDirectory asterisk plugin configuration)             	- 1.3.6.1.4.1.38414.19.2.1
 gotoDevice                    	(GOto - contains environment settings)                      	- 1.3.6.1.4.1.10098.1.2.1.42
 gotoWorkstation               	(GOto - Gonicus Terminal Concept, objectclass)              	- 1.3.6.1.4.1.10098.1.2.1.30
 gosaUserTemplate              	(GOsa - Class for GOsa User Templates)                      	- 1.3.6.1.4.1.10098.1.2.1.19.11
 gotoPrinter                   	(GOto - Gonicus Terminal Concept, objectclass)              	- 1.3.6.1.4.1.10098.1.2.1.31
 gotoTerminal                  	(GOto - Gonicus Terminal Concept, objectclass)              	- 1.3.6.1.4.1.10098.1.2.1.1
 gotoWorkstationTemplate       	(GOto - Gonicus Terminal Concept, objectclass)              	- 1.3.6.1.4.1.10098.1.2.1.34
```

The check-deprecated option will output a list of dn using old attributes and objectClasses of they are present in your ldap server

```shell
fusiondirectory-setup --check-deprecated
List LDAP entries using deprecated attributes or objectclasses
There are no entries in the LDAP using obsolete attributes
There are no entries in the LDAP using obsolete classes
```

The ldif-deprecated option will output an ldif file on the console that you can use with ldapmodify to clean you ldap server from old attributes.

fusiondirectory-setup --ldif-deprecated > remove_deprecated.ldif

If they are old objectClasses it will warn you and you will have to remove them by hand, they have been specified at the check-deprecated step.

Please read it carefully before applying :!::!:

### Checking your indexed attributes

Check that all you index still match with valid attributes present in your ldap directory 

### New format for repository service

  * If you have a repository service.

Open and save it back so it will use the new format


## Migrate FusionDirectory from 1.1 to 1.1.1

### New Depot Configuration

! The repositories have been cleaned and reorganized please update your configuration accordingly [repository organisation][repo-urls] !

### Upgrade schema2ldif

For Jessie distribution add extra repository

```shell
# fusiondirectory debian-extra repository
deb http://repos.fusiondirectory.org/fusiondirectory-extra/debian-jessie/ jessie main
```

* Update the package list and upgrade schema2ldif

```shell
apt-get update
apt-get install schema2ldif
```

### Upgrade FusionDirectory first

  * Upgrade FusionDirectory core package before other ones to avoid dependencies errors:

```shell
apt-get install fusiondirectory
```

  * Upgrade FusionDirectory schema package too.

```shell
apt-get install fusiondirectory-schema
```

## Migrate FusionDirectory from 1.1.1 to 1.2

### Upgrade schema2ldif

For Jessie distribution add extra repository

```shell
# fusiondirectory debian-extra repository
deb http://repos.fusiondirectory.org/fusiondirectory-extra/debian-jessie/ jessie main
```

For Stretch distribution add extra repository

```shell
# fusiondirectory debian-extra repository
deb http://repos.fusiondirectory.org/fusiondirectory-extra/debian-stretch/ stretch main
```

* Update the package list and upgrade schema2ldif

```shell
apt-get update
apt-get install schema2ldif
```

### Upgrade FusionDirectory first

  * Upgrade FusionDirectory core package before other ones to avoid dependencies errors:

```shell
apt-get install fusiondirectory
```

  * Upgrade FusionDirectory schema package too.

```shell
apt-get install fusiondirectory-schema
```

### Upgrade of your LDAP directory

  * Upgrade core schema of FusionDirectory

```shell
fusiondirectory-insert-schema -m /etc/ldap/schema/fusiondirectory/core-fd.schema
```

  * if you are using the renater plugin you have to add its new schema

```shell
fusiondirectory-insert-schema -m /etc/ldap/schema/fusiondirectory/renater-partage-fd.schema
```

  * if you are using the sympa plugin you have to update its schema

```shell
fusiondirectory-insert-schema -i /etc/ldap/schema/fusiondirectory/sympa-fd-conf.schema
fusiondirectory-insert-schema -m /etc/ldap/schema/fusiondirectory/sympa-fd.schema
```

* In case you use system template that have a DNS tab, you need to remake it because the separator "|" is changed to "^"


## Migrate FusionDirectory from 1.2 to 1.2.1

### Upgrade schema2ldif

For Jessie distribution add extra repository

```shell
# fusiondirectory debian-extra repository
deb http://repos.fusiondirectory.org/fusiondirectory-extra/debian-jessie/ jessie main
```

For Stretch distribution add extra repository

```shell
# fusiondirectory debian-extra repository
deb http://repos.fusiondirectory.org/fusiondirectory-extra/debian-stretch/ stretch main
```


* Update the package list and upgrade schema2ldif

```shell
apt-get update
apt-get install schema2ldif
```

### Upgrade FusionDirectory first

  * Upgrade FusionDirectory core package before other ones to avoid dependencies errors:

```shell
apt-get install fusiondirectory
```

  * Upgrade FusionDirectory schema package too.

```shell
apt-get install fusiondirectory-schema
```


## Migrate FusionDirectory from 1.2.1 to 1.2.2

### Upgrade FusionDirectory first

  * Upgrade FusionDirectory core package before other ones to avoid dependencies errors:

```shell
apt-get install fusiondirectory
```

  * Upgrade FusionDirectory schema package too.

```shell
apt-get install fusiondirectory-schema
```

## Migrate FusionDirectory from 1.2.2 to 1.2.3

### Upgrade FusionDirectory first

  * Upgrade FusionDirectory core package before other ones to avoid dependencies errors:

```shell
apt-get install fusiondirectory
```

  * Upgrade FusionDirectory schema package too.

```shell
apt-get install fusiondirectory-schema
```


## Migrate FusionDirectory from 1.2.3 to 1.3

### Upgrade FusionDirectory first

Upgrade FusionDirectory core package before other ones to avoid
dependencies errors:

``` {.sourceCode .shell}
apt-get install fusiondirectory
```

Upgrade FusionDirectory schema package too.

``` {.sourceCode .shell}
apt-get install fusiondirectory-schema
```

### Upgrade of LDAP directory


Upgrade core schema of FusionDirectory

``` {.sourceCode .shell}
fusiondirectory-insert-schema -m /etc/ldap/schema/fusiondirectory/core-fd-conf.schema
```

if you are using the audit plugin you have to update is schema

``` {.sourceCode .shell}
fusiondirectory-insert-schema -m /etc/ldap/schema/fusiondirectory/audit-fd.schema
```

if you are using the argonaut plugin you have to update is schema

``` {.sourceCode .shell}
fusiondirectory-insert-schema -m /etc/ldap/schema/fusiondirectory/argonaut-fd.schema
```

if you are using the community plugin you have to update is schema

``` {.sourceCode .shell}
fusiondirectory-insert-schema -m /etc/ldap/schema/fusiondirectory/community-fd.schema
```

if you are using the dhcp plugin you have to update is schema

``` {.sourceCode .shell}
fusiondirectory-insert-schema -m /etc/ldap/schema/fusiondirectory/dhcp-fd.schema
```

if you are using the ejbca plugin you have to update is schema

``` {.sourceCode .shell}
fusiondirectory-insert-schema -m /etc/ldap/schema/fusiondirectory/ejbca-fd.schema
```

if you are using the opsi plugin you have to update is schema

``` {.sourceCode .shell}
fusiondirectory-insert-schema -m /etc/ldap/schema/fusiondirectory/opsi-fd.schema
```

if you are using the postfix plugin you have to update is schema

``` {.sourceCode .shell}
fusiondirectory-insert-schema -m /etc/ldap/schema/fusiondirectory/postfix-fd.schema
```

if you are using the supann plugin you have to update is schema

``` {.sourceCode .shell}
fusiondirectory-insert-schema -m /etc/ldap/schema/fusiondirectory/supann-fd-conf.schema
```

if you are using the systems plugin you have to update is schema

``` {.sourceCode .shell}
fusiondirectory-insert-schema -m /etc/ldap/schema/fusiondirectory/systems-fd.schema

fusiondirectory-insert-schema -m /etc/ldap/schema/fusiondirectory/systems-fd-conf.schema
```

if you are using the user reminder plugin you have to update is schema

``` {.sourceCode .shell}
fusiondirectory-insert-schema -m /etc/ldap/schema/fusiondirectory/user-reminder-fd-conf.schema
```

if you are using the weblink plugin you have to update is schema

``` {.sourceCode .shell}
fusiondirectory-insert-schema -m /etc/ldap/schema/fusiondirectory/weblink-fd.schema
```

Migrate FusionDirectory from 1.3 to 1.3.1
=========================================

**The gpg keys for FusionDirectory and Argonaut have been renewed so you
need to install the new keys for the packages to install correctly**

Upgrade FusionDirectory first
-----------------------------

Upgrade FusionDirectory core package before other ones to avoid
dependencies errors:

``` {.sourceCode .shell}
apt-get install fusiondirectory
```

Upgrade FusionDirectory schema package too.

``` {.sourceCode .shell}
apt-get install fusiondirectory-schema
```

Upgrade of LDAP directory
-------------------------

if you are using the sinaps plugin you have to update is schema

``` {.sourceCode .shell}
fusiondirectory-insert-schema -m /etc/ldap/schema/fusiondirectory/sinaps-fd-conf.schema
```

Fonctionality Added
-------------------

-   The macro **%passwordClear%** is now available in the sambaAccount
    so that in can be used in hooks for AD sync.
-   The ldapdump fonctionality is now available on the configuration
-   The supann etablissment objects is now equal to entite objects
    except for the attribute ou

Fonctionality removed
---------------------

-   The google+ account storage has been removed from personal social
    accounts tab



[php-cas]: http://packages.ubuntu.com/trusty/all/php-cas/download

[repo-urls]: https://fusiondirectory-user-manual.readthedocs.io/en/1.4/fusiondirectory/install/index.html

---
* Further information

To improve this piece of software, please report all kind of errors using the bug tracker
on https://gitlab.fusiondirectory.org

Documentation: https://fusiondirectory-user-manual.readthedocs.io/en/1.4/fusiondirectory/index.html
Mailinglist:   https://lists.fusiondirectory.org/
Irc:           #fusiondirectory on freenode

---
The FusionDirectory project https://www.fusiondirectory.org/



Enjoy :)

