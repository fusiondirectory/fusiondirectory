
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

## Get help

There are a couple ways you can try [to get help][get help].You can also join the `#fusiondirectory` IRC channel at freenode.net.

You can [register on our system][register] and enter your bug [on the forge][issues-forge] or [here at github][issues-github] even if the forge is the prefered way of dealing with bugs

## IRC Etiquette

* If we don't answer right away then just hang out in the channel.  Someone will
  eventually write back to you as it just means we are away from keyboard,
  working on something else, or in a different timezone than you.
* You should treat IRC as what it is: asynchronous chat.  Sure the messages can
  be instant but in most channels people are in different time zones.  At times
  chat replies can be in excess of 24hrs.
  
## Donate

If you like [FusionDirectory][FusionDirectory] and would like to [donate][donate-liberapay] even a small amount you can go to our Liberapay account
  
## License

[FusionDirectory][FusionDirectory] is  [GPL 2 License](COPYING).

[FusionDirectory]: https://www.fusiondirectory.org/

[get help]: https://www.fusiondirectory.org/contact-us/

[register]: https://register.fusiondirectory.org

[issues-forge]: https://forge.fusiondirectory.org/

[issues-github]: https://github.com/fusiondirectory/fusiondirectory/issues

[donate-liberapay]: https://liberapay.com/fusiondirectory/donate
