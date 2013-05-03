<table summary="" style="width:100%; vertical-align:top; text-align:left;" cellpadding="4" border="0">

 <!-- Headline container -->
 <tr>
  <td colspan="4">
   <h2><img alt="" align="middle" src="plugins/generic/images/head.png" class="center"> {t}Personal information{/t}</h2>
  </td>
 </tr>

 <!-- Base 1 container -->
 <tr>

  <!-- Image container -->
  <!-- Image container -->
  <td style="vertical-align:top">
    <table>
     <tr>
        <td style='width:150px; height:200px; background-color:gray; vertical-align: middle; text-align: center;'>

    {if !$userPicture_is_readable}
      <img class='center' border="0" width="100%" src="plugins/users/images/default.jpg" alt="{t}Personal picture{/t}">
    {else}
            <img  src="getbin.php?rand={$rand}" alt='' style='max-width:150px; max-height: 200px; vertical-align: middle;'
              alt="{t}Personal picture{/t}" >
    {/if}
    </td>
     </tr>
     <tr>
    <td>
{render acl=$userPictureACL checked=$use_edit_picture}
      <input style='vertical-align:top;' type="submit" name="edit_picture" value="{t}Change picture{/t}...">
{/render}
    </td>
     </tr>
    </table>
  </td>
  <!--
  Old image container, with incorrect image scaling.
  <td style="vertical-align:top">
   <img align="bottom" border="1" width="145" height="200" src="getbin.php?rand={$rand}" alt="{t}Personal picture{/t}">
   <br>
   <input type="submit" name="edit_picture" value="{t}Change picture{/t}...">
  </td>
  -->

  <!-- Name, ... -->
  <td style="vertical-align:top;">
   <table summary="">
{if $is_template ne "true"}
    <tr>
     <td><label for="sn">{t}Last name{/t}{$must}</label></td>
     <td>
{render acl=$snACL}
   <input type="text" id="sn" name="sn" size=25 maxlength=60 value="{$sn}">
{/render}
   </td>
    </tr>
{else}
    <tr>
     <td><label for="sn">{t}Template name{/t}{$must}</label></td>
     <td>{render acl=$snACL}<input type="text" id="sn" name="sn" size=25 maxlength=60 value="{$sn}">{/render}</td>
    </tr>
{/if}

{if $is_template ne "true"}
    <tr>
     <td><label for="givenName">{t}First name{/t}{$must}</label></td>
     <td>

{render acl=$givenNameACL}
   <input type="text" id="givenName" name="givenName" size=25 maxlength=60 value="{$givenName}">
{/render}
   </td>
    </tr>
{/if}
    <tr>
     <td><label for="uid">{t}Login{/t}{if $is_template ne "true"}{$must}{/if}</label></td>
     <td>
{render acl=$uidACL}
      <input type="text" id="uid" name="uid" size=25 maxlength=60  value="{$uid}">
{/render}
   </td>
    </tr>

    <tr>
     <td>
      <div style="height:10px;"></div>
      <label for="personalTitle">{t}Personal title{/t}</label>
     </td>
     <td>
      <div style="height:10px;"></div>
{render acl=$personalTitleACL checked=$use_personalTitle}
    <input type="text" id="personalTitle" name="personalTitle" size=25 maxlength=60 value="{$personalTitle}">
{/render}
     </td>
    </tr>
    <tr>
     <td><label for="academicTitle">{t}Academic title{/t}</label></td>
     <td>
{render acl=$academicTitleACL checked=$use_academicTitle}
        <input type="text" id="academicTitle" name="academicTitle" size=25 maxlength=60 value="{$academicTitle}">
{/render}
     </td>
    </tr>
    <tr>
     <td>
      <div style="height:10px;"></div>
      <label for="day">{t}Date of birth{/t}</label>
     </td>
     <td>
      <div style="height:10px;"></div>
{render acl=$dateOfBirthACL}
        <input type="text" id="dateOfBirth" name="dateOfBirth" class="date" style='width:100px' value="{$dateOfBirth}">
        {if $dateOfBirthACL|regex_replace:"/[cdmr]/":"" == "w"}
        <script type="text/javascript">
          {literal}
          var datepicker  = new DatePicker({ relative : 'dateOfBirth', language : '{/literal}{$lang}{literal}', keepFieldEmpty : true,
                                             enableCloseEffect : false, enableShowEffect : false });
          {/literal}
        </script>
        {/if}
{/render}
     </td>
    </tr>
    <tr>
     <td>
      <label for="gender">{t}Sex{/t}</label>
     </td>
     <td>
{render acl=$genderACL}
        <select size="1" id="gender" name="gender">
         {html_options options=$gender_list selected=$gender}
        </select>
{/render}
     </td>
    </tr>
    <tr>
     <td>
      <label for="preferredLanguage">{t}Preferred langage{/t}</label>
     </td>
     <td>
{render acl=$preferredLanguageACL checked=$use_preferredLanguage}
        <select size="1" id="preferredLanguage" name="preferredLanguage">
         {html_options options=$preferredLanguage_list selected=$preferredLanguage}
        </select>
{/render}
     </td>
    </tr>
    <tr>
     <td>
      <label for="description">{t}Description{/t}</label>
     </td>
     <td>
{render acl=$descriptionACL}
      <input type="text" id="description" name="description" size=35 maxlength=80 value="{$description}"/>
{/render}
     </td>
    </tr>
    <tr>
     <td>
      <label for="base">{t}Base{/t}</label>
     </td>
     <td>
{render acl=$baseACL checked=$use_base}
      {$base}
{/render}
     </td>
    </tr>
   </table>
  </td>
  <td style="border-left:1px solid #A0A0A0">
   &nbsp;
  </td>
  <td style="vertical-align:top;">

   <!-- Address, ... -->
   <table summary="" style="width:100%">
    <tr>
     <td style="vertical-align:top;"><label for="homePostalAddress">{t}Address{/t}</label></td>
      <td>
{render acl=$homePostalAddressACL checked=$use_homePostalAddress}
       <textarea id="homePostalAddress" name="homePostalAddress" rows="3" style="width:100%">{$homePostalAddress}</textarea>
{/render}
     </td>
    </tr>
    <tr>
     <td><label for="homePhone">{t}Private phone{/t}</label>
     </td><td>
{render acl=$homePhoneACL checked=$use_homePhone}
       <input type="text" id="homePhone" name="homePhone" size=25 maxlength=60 value="{$homePhone}">
{/render}
     </td>
    </tr>
    <tr>
     <td><label for="labeledURI">{t}Homepage{/t}</label></td>
     <td>
{render acl=$labeledURIACL checked=$use_labeledURI}
    <input type="text" id="labeledURI" name="labeledURI" size=25 maxlength=60 value="{$labeledURI}">
{/render}
     </td>
    </tr>
    <tr>
     <td colspan=2>
      <div style="height:10px;"></div>
     </td>
    </tr>
    <tr>
     <td><label for="pw_storage">{t}Password storage{/t}</label></td>
     <td>
{render acl=$passwordStorageACL checked=$use_pw_storage}
        <select size="1" id="pw_storage" name="pw_storage" onchange='document.mainform.submit()'{$disabled_pw_storage}>
      {foreach from=$pwmode key=value item=desc}
        <option value="{$value}" {if $pwmode_select == $value}selected{/if} >{$desc}</option>
      {/foreach}
        </select>
  {if $pw_configurable eq "true"}
        &nbsp; <input id="edit_pw_method" type="submit" name="edit_pw_method" value="{t}Configure{/t}...">
  {/if}
{/render}
     </td>
    </tr>
  {if $is_template}
    <tr>
      <td><label for="default_pw">Default password</label></td>
      <td>
        <input id="default_pw" type="text" name="default_pw" value="{$default_pw}"/>
      </td>
    </tr>
  {/if}

    <tr>
     <td colspan=2>
      <div style="height:10px;"></div>
     </td>
    </tr>
    <tr>
     <td style='vertical-align:top'><label for="edit_perms">{t}Restrict login to{/t}</label></td>
     <td>
        {render acl=$gosaLoginRestrictionACL}
          {$gosaLoginRestrictionWidget}
        {/render}
        {render acl=$gosaLoginRestrictionACL}
          <input type="text" id="res" name="res" size="22" maxlength="33"
            value="{t}IP or network{/t}" onFocus='document.getElementById("res").value=""'/>
        {/render}
        {render acl=$gosaLoginRestrictionACL}
           <input id="add_res" type="submit" name="add_res" value="{t}Add{/t}"/>
        {/render}
     </td>
    </tr>
   </table>
  </td>
 </tr>
</table>

<p class="seperator">&nbsp;</p>

<table summary="" class="line" style="width:100%; vertical-align:top; text-align:left;" cellpadding="4" border="0">

 <!-- Headline container -->
 <tr>
  <td colspan="3">
   <h2><img alt="" align="middle" src="plugins/generic/images/house.png" class="center">&nbsp;{t}Organizational information{/t}</h2>
  </td>
 </tr>

 <!-- Base 2 container -->
 <tr>

  {if $governmentmode ne "true"}
  <!-- Company, ... -->
  <td style="vertical-align:top;">
   <table summary="">
    <tr>
     <td><label for="o">{t}Organization{/t}</label></td>
     <td>
{render acl=$oACL checked=$use_o}
    <input type="text" id="o" name="o" size=22 maxlength=60 value="{$o}">
{/render}
     </td>
    </tr>
    <tr>
     <td><label for="ou">{t}Department{/t}</label></td>
     <td>
{render acl=$ouACL checked=$use_ou}
    <input type="text" id="ou" name="ou" size=22 maxlength=60 value="{$ou}">
{/render}
     </td>
    </tr>
    <tr>
     <td><label for="departmentNumber">{t}Department No.{/t}</label></td>
     <td>
{render acl=$departmentNumberACL checked=$use_departmentNumber}
        <input type="text" id="departmentNumber" name="departmentNumber" size=22 maxlength=60 value="{$departmentNumber}">
{/render}
     </td>
    </tr>
    <tr>
     <td><label for="employeeNumber">{t}Employee No.{/t}</label></td>
     <td>
{render acl=$employeeNumberACL checked=$use_employeeNumber}
          <input type="text" id="employeeNumber" name="employeeNumber" size=22 maxlength=60 value="{$employeeNumber}">
{/render}
     </td>
    </tr>
    <tr>
     <td><label for="employeeType">{t}Employee type{/t}</label></td>
     <td>
{render acl=$employeeTypeACL checked=$use_employeeType}
          <input type="text" id="employeeType" name="employeeType" size=22 maxlength=60 value="{$employeeType}">
{/render}
     </td>
    </tr>
    <tr>
     <td><label for="manager">{t}Manager{/t}</label></td>
     <td>
{render acl=$managerACL}
        <input type='text' name='manager_name' id='manager_name' value='{$manager_name}' disabled
          title='{$manager}'>
{/render}
{render acl=$managerACL}
        <input type='image' src='images/lists/edit.png' name='editManager' class='center'>
{/render}
        {if $manager!=""}
{render acl=$managerACL}
        <img src='images/info_small.png' title='{$manager}' class='center'>
{/render}
{render acl=$managerACL}
        <input type='image' src='images/lists/trash.png' name='removeManager' class='center'>
{/render}
        {/if}
     </td>
    </tr>
   </table>
  </td>

  <td style="border-left:1px solid #A0A0A0">
   &nbsp;
  </td>

  <!-- Phone, ... -->
  <td style="vertical-align:top;">
   <table summary="">
    <tr>
     <td><label for="roomNumber">{t}Room No.{/t}</label></td>
     <td>
{render acl=$roomNumberACL checked=$use_roomNumber}
    <input type="text" id="roomNumber" name="roomNumber" size=22 maxlength=60 value="{$roomNumber}">
{/render}
     </td>
    </tr>
    {if $has_phoneaccount ne "true"}
    <tr>
     <td><label for="telephoneNumber">{t}Phone{/t}</label></td>
     <td>
{render acl=$telephoneNumberACL checked=$use_telephoneNumber}
          <input type="text" id="telephoneNumber" name="telephoneNumber" size=22 maxlength=60 value="{$telephoneNumber}"/>
        {if $phone_dialog_available}
          <input type='image' src='images/lists/edit.png' name='editPhone' class='center'/>
        {/if}
{/render}
     </td>
    </tr>
    {/if}
    <tr>
     <td><label for="mobile">{t}Mobile{/t}</label></td>
     <td>
{render acl=$mobileACL checked=$use_mobile}
    <input type="text" id="mobile" name="mobile" size=22 maxlength=60 value="{$mobile}"/>
        {if $mobile_dialog_available}
          <input type='image' src='images/lists/edit.png' name='editMobile' class='center'/>
        {/if}
{/render}
     </td>
    </tr>
    <tr>
     <td><label for="pager">{t}Pager{/t}</label></td>
     <td>
{render acl=$pagerACL checked=$use_pager}
          <input type="text" id="pager" name="pager" size=22 maxlength=60 value="{$pager}">
{/render}
     </td>
    </tr>
    <tr>
     <td><label for="facsimileTelephoneNumber">{t}Fax{/t}</label></td>
     <td>
{render acl=$facsimileTelephoneNumberACL checked=$use_facsimileTelephoneNumber}
    <input type="text" id="facsimileTelephoneNumber" name="facsimileTelephoneNumber" size=22 maxlength=60 value="{$facsimileTelephoneNumber}">
{/render}
     </td>
    </tr>
   </table>
  </td>

  <td style="border-left:1px solid #A0A0A0">
   &nbsp;
  </td>

  <!-- Location, ... -->
  <td style="vertical-align:top;">
   <table summary="" style="width:100%">
    <tr>
     <td><label for="l">{t}Location{/t}</label></td>
     <td>
{render acl=$lACL checked=$use_l}
    <input type="text" id="l" name="l" size=22 maxlength=60 value="{$l}">
{/render}
     </td>
    </tr>
    <tr>
     <td><label for="st">{t}State{/t}</label></td>
     <td>
{render acl=$stACL checked=$use_st}
    <input type="text" id="st" name="st" size=22 maxlength=60 value="{$st}">
{/render}
     </td>
    </tr>
    <tr>
     <td style="vertical-align:top;"><label for="postalAddress">{t}Address{/t}</label></td>
     <td>
{render acl=$postalAddressACL checked=$use_postalAddress}
          <textarea id="postalAddress" name="postalAddress" rows=3 style="width:100%">{$postalAddress}</textarea>
{/render}
     </td>
    </tr>
   </table>
  </td>

  {else}

  <!-- Beschreibung, ... -->
  <td style="vertical-align:top;border-right:1px solid #b0b0b0">
   <table summary="">
    <tr>
     <td><label for="vocation">{t}Vocation{/t}</label></td>
     <td>
{render acl=$vocationACL checked=$use_vocation}
    <input type="text" id="vocation" name="vocation" size=22 maxlength=60 value="{$vocation}">
{/render}
     </td>
    </tr>
    <tr>
     <td><label for="gouvernmentOrganizationalUnitDescription">{t}Unit description{/t}</label></td>
     <td>
{render acl=$gouvernmentOrganizationalUnitDescriptionACL checked=$use_gouvernmentOrganizationalUnitDescription}
          <input type="text" id="gouvernmentOrganizationalUnitDescription" name="gouvernmentOrganizationalUnitDescription" size=22 maxlength=60
    value="{$gouvernmentOrganizationalUnitDescription}">
{/render}
     </td>
    </tr>
    <tr>
     <td><label for="gouvernmentOrganizationalUnitSubjectArea">{t}Subject area{/t}</label></td>
     <td>
{render acl=$gouvernmentOrganizationalUnitSubjectAreaACL checked=$use_gouvernmentOrganizationalUnitSubjectArea}
          <input type="text" id="gouvernmentOrganizationalUnitSubjectArea" name="gouvernmentOrganizationalUnitSubjectArea" size=22 maxlength=60
            value="{$gouvernmentOrganizationalUnitSubjectArea}">
{/render}
     </td>
    </tr>
    <tr>
     <td><label for="functionalTitle">{t}Functional title{/t}</label></td>
     <td>
{render acl=$functionalTitleACL checked=$use_functionalTitle}
          <input type="text" name="functionalTitle" id="functionalTitle"  size=22 maxlength=60
              value="{$functionalTitle}">
{/render}
     </td>
    </tr>
    <tr>
     <td><label for="role">{t}Role{/t}</label></td>
     <td>
{render acl=$roleACL checked=$use_role}
          <input type="text" id="role" name="role" size=22 maxlength=60 value="{$role}">
{/render}
     </td>
    </tr>
   </table>
  </td>

  <!-- Phone, ... -->
  <td style="vertical-align:top;border-right:1px solid #b0b0b0">
   <table summary="">
    <tr>
     <td><label for="gouvernmentOrganizationalPersonLocality">{t}Person locality{/t}</label></td>
     <td>
{render acl=$gouvernmentOrganizationalPersonLocalityACL checked=$use_gouvernmentOrganizationalPersonLocality}
          <input type="text" id="gouvernmentOrganizationalPersonLocality" name="gouvernmentOrganizationalPersonLocality" size=22 maxlength=60
                value="{$gouvernmentOrganizationalPersonLocality}">
{/render}
     </td>
    </tr>
    <tr>
     <td><label for="gouvernmentOrganizationalUnit">{t}Unit{/t}</label></td>
     <td>
{render acl=$gouvernmentOrganizationalUnitACL checked=$use_gouvernmentOrganizationalUnit}
    <input type="text" id="gouvernmentOrganizationalUnit" name="gouvernmentOrganizationalUnit" size=22 maxlength=60
              value="{$gouvernmentOrganizationalUnit}">
{/render}
     </td>
    </tr>
    <tr>
     <td><label for="street">{t}Street{/t}</label></td>
     <td>
{render acl=$streetACL checked=$use_street}
          <input type="text" name="street" id="street" size=22 maxlength=60 value="{$street}">
{/render}
     </td>
    </tr>
    <tr>
     <td><label for="postalCode">{t}Postal code{/t}</label></td>
     <td>
{render acl=$postalCodeACL checked=$use_postalCode}
          <input type="text" name="postalCode" id="postalCode" size=22 maxlength=60 value="{$postalCode}">
{/render}
     </td>
    </tr>
    <tr>
     <td><label for="houseIdentifier">{t}House identifier{/t}</label></td>
     <td>
{render acl=$houseIdentifierACL checked=$use_houseIdentifier}
          <input type="text" id="houseIdentifier" name="houseIdentifier" size=22 maxlength=60
              value="{$houseIdentifier}">
{/render}
     </td>
    </tr>
   </table>
  </td>

  <!-- Location, ... -->
  <td style="vertical-align:top;">
   <table summary="">
    <tr>
     <td><label for="roomNumber">{t}Room No.{/t}</label></td>
     <td>
{render acl=$roomNumberACL checked=$use_roomNumber}
    <input type="text" id="roomNumber" name="roomNumber" size=22 maxlength=60 value="{$roomNumber}">
{/render}
     </td>
    </tr>
    <tr>
     <td><label for="telephoneNumber">{t}Phone{/t}</label></td>
     <td>
{render acl=$telephoneNumberACL checked=$use_telephoneNumber}
        {if $has_phoneaccount ne "true"}
      <input type="text" id="telephoneNumber" name="telephoneNumber" size=22 maxlength=60 value="{$telephoneNumber}">
        {else}
      <i>{t}Please use the phone tab{/t}</i>
        {/if}
{/render}
     </td>
    </tr>
    <tr>
     <td><label for="facsimileTelephoneNumber">{t}Fax{/t}</label></td>
     <td>
{render acl=$facsimileTelephoneNumberACL checked=$use_facsimileTelephoneNumber}
          <input type="text" id="facsimileTelephoneNumber" name="facsimileTelephoneNumber" size=22 maxlength=60
                value="{$facsimileTelephoneNumber}">
{/render}
     </td>
    </tr>
    <tr>
     <td><label for="ivbbLastDeliveryCollective">{t}Last delivery{/t}</label></td>
     <td>
{render acl=$ivbbLastDeliveryCollectiveACL checked=$use_ivbbLastDeliveryCollective}
          <input type="text" name="ivbbLastDeliveryCollective" size=22 maxlength=60 id="ivbbLastDeliveryCollective"
              value="{$ivbbLastDeliveryCollective}">
{/render}
     </td>
    </tr>
    <tr>
     <td><label for="publicVisible">{t}Public visible{/t}</label></td>
     <td>
{render acl=$publicVisibleACL checked=$use_publicVisible}
        <select size="1" id="publicVisible" name="publicVisible">
         {html_options values=$ivbbmodes output=$ivbbmodes selected=$publicVisible}
        </select>
{/render}
     </td>
    </tr>
   </table>
  </td>
 {/if}
 </tr>
</table>

<input type=hidden name="generic">

<!-- Place cursor -->
<script language="JavaScript" type="text/javascript">
  <!-- // First input field on page
  focus_field('sn');
  -->
</script>
