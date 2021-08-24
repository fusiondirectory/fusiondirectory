
# FusionDirectory

[FusionDirectory][FusionDirectory] provides a solution to daily management of data stored in an LDAP directory. Becoming the cornerstone of the information system, the corporate directory becomes more complex offering more data and managing more infrastructure services.

This interface is simple and can be used to delegate fully or partly the data management to non-specialists.

## Features

* Users, groups, mail, sudo, ssh, systems, services management, dhcp, dns 
* Complex Roles Management :
  * ACLs are only used by FusionDirectory and are not intrusive to other applications using the directory server.
  * ACLs allow fine grained control to who can do what into FusionDirectory
  * ACLs can be assigned to roles. We may have a role
    * User : it can connect to FusionDirectory with his login / password to change is data only when permitted by the admin.
    * Local administrator : this role will be able to manage users and groups and also a branch.
    * global administrator : this role has the right to do everything.
    * human resources : this role can only create users from template to optimize the flow of arrival of new people.
* Access to multiple LDAP trees
* FusionDirectory Triggers
  * FusionDirectory incorporates a series of triggers that can launch a specific action based on a task FusionDirectory must run.
  * These triggers are associated with a content type (LDAP user, group, server, password, service (etc. ..) and the triggering action (create, edit, delete, change password … )
  * This system is very useful when certain actions should be followed on arrival or departure of a person in the company For example, when creating a user, a script generation form can be executed automatically with information from the LDAP server.
  * This can be useful for generating badges with photo, a form of access to the canteen or sending an email to warn of the actual arrival of the person.
  * This system is also convenient when we want to deploy the account of that person on an application does not support LDAP (FusionDirectory can also transmit the password) Another example is when a user leaves, you must:
  * archive and delete his mailbox
  * archive and remove its network space
  * delete him from third party applications not connected to LDAP.
  * All of this can be easily done by shell scripts (at least in UNIX environment) and run automatically after the suppression of the person by the administrator in FusionDirectory

* The interaction with non-LDAP applications

FusionDirectory stores information of a service or a server on an LDAP server.
How about when this service does not have the opportunity to interact with LDAP?

This question can be solved by creating:

* LDAP schema suitable for application to the LDAP server
* A plugin for its management in FusionDirectory with the simple plugin api
* An Argonaut module for the client installed on the server

## Installation and upgrade of FusionDirectory

The installation and upgrade information is available on [Install FusionDirectory][fusiondirectory-install]

## Migration to FusionDirectory

To migrate an existing LDAP tree, you've to do all steps [Install FusionDirectory][fusiondirectory-install],
plus some modifications:

* FusionDirectory only shows users that have the objectClass inetOrgPerson

* FusionDirectory only recognizes subtrees (or departments in FusionDirectory's view of things) that have the objectClass gosaDepartment.
  You can hide subtrees from FusionDirectory by not putting this objectClass inside.

The FusionDirectory setup may be used to do these migrations, but it is not meant
to work in every possible circumstance. Please be carefull when using it on
productive system.

That should be all. Entries should be visible in FusionDirectory now.
Be aware that if your naming policy of user cn's differs from the way FusionDirectory handles it, the entries get rewritten to a FusionDirectory style dn.

## Get help

### Community support

There are a couple ways you can try [to get help][get help].You can also join the `#fusiondirectory` IRC channel at libera.chat.

### Professional support

Professional support is provided through of subscription.

We have two type of subscription :

* [FusionDirectory][subscription-fusiondirectory] : Global subscription for FusionDirectory and all the plugins
* [FusionDirectory Plus][subscription-fusiondirectory-plus] : Expert Support on Education, Deployement and Infrastructure plugins

The subscription provides access to FusionDirectory's stable enterprise repository, providing reliable software updates and security enhancements,
as well as technical help and support.

Choose the plan that's right for you. Our subscriptions are flexible and scalable according to your needs

The subscription period is one year from the date of purchase and gives you access to the extensive infrastructure of enterprise-class software and services.

## IRC Etiquette

* If we don't answer right away then just hang out in the channel.  Someone will
  eventually write back to you as it just means we are away from keyboard,
  working on something else, or in a different timezone than you.

* You should treat IRC as what it is: asynchronous chat.  Sure the messages can
  be instant but in most channels people are in different time zones.  At times
  chat replies can be in excess of 24hrs.

### Best practice badge

[![CII Best Practices](https://bestpractices.coreinfrastructure.org/projects/351/badge)](https://bestpractices.coreinfrastructure.org/projects/351)
  
## Crowfunding

If you like us and want to send us a small contribution you can use the following crowfunding services

* [donate-liberapay]

* [donate-kofi]

* [donate-opencollective]

* [donate-communitybridge]
  
## License

[FusionDirectory][FusionDirectory] is  [GPL 2 License](COPYING).

[FusionDirectory]: https://www.fusiondirectory.org/

[fusiondirectory-install]: https://fusiondirectory-user-manual.readthedocs.io/en/1.4/fusiondirectory/install/index.html

[get help]: https://www.fusiondirectory.org/en/communaute/

[subscription-fusiondirectory]: https://www.fusiondirectory.org/en/subscription-fusiondirectory/

[subscription-fusiondirectory-plus]: https://www.fusiondirectory.org/en/subscriptions-fusiondirectory-plus/

[register]: https://register.fusiondirectory.org

[donate-liberapay]: https://liberapay.com/fusiondirectory/donate

[donate-kofi]: https://ko-fi.com/fusiondirectory

[donate-opencollective]: https://opencollective.com/fusiondirectory

[donate-communitybridge]: https://funding.communitybridge.org/projects/fusiondirectory
