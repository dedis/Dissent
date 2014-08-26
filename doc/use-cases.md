# Dissent Use Cases #

This document outlines use cases for which Dissent's anonymity protocol
is expected to be suitable and potentially deployable.
Its immediate purpose is to clarify intended use cases for Dissent
for SAFER program analysis and assessment purposes.
It is only a preliminary draft and is expected to evolve.

## Use Case 1: NGOs in Repressistan ##


### Scenario ###

In the state of Repressistan,
a human-rights non-governmental organization (NGO) is attempting to operate,
and is formally approved and accepted by the local government
but informally is kept under heavy surveillance and sometimes harrassment.
Most of the NGO's personnel are not normally, ``a priori''
of particular individual interest to the semi-hostile state.
Perhaps for no other reason than ``being foreign,''
however,
the NGO has reasons to fear state-sanctioned surveillance,
cyber-attacks targetted at their individual members,
and/or infiltration by planted agents or corrupted group members,
for purposes such as extracting sensitive information
or otherwise harrassing the group.
In addition, individual group members may occasionally --
either intentionally or unintentionally --
send internal messages or otherwise engage in actions
nominally internal to the group that,
if known to the state and linkable to the individual group member,
may place that member at particular risk of heightened surveillance,
harassment by state-sanctioned parties,
targeted cyber-attacks based relying on the attacker's knowledge
of their personally identifying information,
or even legal or quasi-legal threats
of prison or worse under local law.

To protect themselves in this semi-hostile environment,
the NGO's local network administrators work with administrators
back at home set up a Dissent deployment for communication
within and beyond their Repressistan enclave.
Dissent enables individual group members 
to perform as many of their day-to-day communication activities
as possible in a way that protects their individual identities
within the larger group.
For example, when NGO members in Repressistan use their web browsers
to connect to web sites outside the NGO's enclave --
to web sites either in Repressistan or in the ``free world''
for which connections must pass {\em through} a state-controlled ISP --
the NGO's Dissent configuration ensures that these
Web browsing connections are strongly anonymized,
at least within the group, to the extent feasible.
Similarly, if participate members of the group
participate in publishing a blog on the group's activities,
but some content on that blog occasionally risks touching on
topics deemed politically sensitive in Repressistan
and attracting censorship or harrassment attempts by the authorities,
then the NGO members involved in this sensitive speech
can avoid being singled out individually
but gain the cover of the group's collective voice.
Any undesired interest or actions the NGO's individual members
might attract from the state
though these browsing or blogging activities,
ultimately,
the attacker is forced to direct at the group or NGO as a whole,
reducing the risk of placing individual members at personal risk.

### Use Case Details ###

1. The NGO's sysadmins help all of their users and affiliates
in Repressistan generate GPG public/private keypairs for use with Dissent.
(These same GPG keypairs could be used for other more conventional
purposes as well, e.g., sending and receiving non-anonymous
GPG-encrypted E-mail or documents.)
The NGO's sysadmins collect and agree on a list of all the public keys
of users and affiliates authorized to use the NGO's Dissent group.

2. The NGO's sysadmins build and agree on a Dissent group definition listing:

	* The public keys and network addresses of
	at least three Dissent servers,
	one managed independently by each sysadmin.
	Some servers are located in Repressistan,
	while others may be located remotely in Freetopia,
	the NGO's home country.

	* The public keys of all authorized users as described above.

	* Other relevant group configuration and performance tweaking
	parameters.

3. The sysadmins initialize their servers with the agreed-upon group definition,
which is securely identified by a secure hash of the group definition file.

4. The sysadmins give their users instructions for installing the
Dissent client software, and bundle the client software with a copy of
the Dissent group definition, so that the client software can securely
identify the group definition and the servers supporting the group
(as listed in the group definition file).
The users are instructed how to use GPG to verify
that the group definition file has been signed by *all three* NGO sysadmins.

5. The users run their Dissent clients on their laptops or other mobile devices.
In this configuration, the Dissent client sets up a Tor-like anonymous
browsing environment that anonymously tunnels each user's Web browsing session
through one of the Dissent servers (which acts as an "exit relay"),
to whatever remote web site the user wishes to interact with.
All connections from all users at the NGO are all indistinguishable and
appear to be coming from this one exit relay representing the whole NGO.
The Dissent anonymity engine ensures that this indistinguishability
is preserved even if the exit relay server is compromised,
provided that at least one of the *other* Dissent servers is uncompromised.
If the NGO's sysadmins place at least one of the servers remotely
at the NGO's home office in Freetopia,
this provides a stronger anonymity defense against physical server compromise
at their Repressistan enclave.


## Use Case 2: Wide-Area Discussion Forums ##

### Scenario ###

Again within a RATS,
a moderately large group of ordinary Internet users,
who are subjects of and living in a RATS' domain,
wish to use a Twitter-like forum to discuss, report on, or debate
general social, cultural, or other largely sanctioned issues.
The RATS generally tolerates much of this discussion.
Occasionally, however, a few of these users
might accidentally or intentionally touch on
topics ``taboo'' or sensitive to the RATS.
If (as is likely) the RATS has infiltrated this group,
the RATS might attempt to single out individual members for retribution,
or if they are unable to do so,
simply to disrupt the group --
by blocking members it can identify for example.

To protect their political speech, therefore,
these Repressistan citizens employ existing
point-to-point anonymization and unobservability/unblockability
technologies such as Tor and StegoTorus 
to connect with a Twitter-like forum run as a Dissent group
hosted {\em outside of} Repressistan.
The use of existing point-to-point communication tools
offers these users some protection against being
censored, harrassed, or jailed by the RATS for their speech,
but do not offer strong protection against traffic analysis
or intersection attacks by the Repressistan authorities.
By connecting to and using a Dissent-based microblogging forum
outside of Repressistan, however,
this Dissent forum offers individual Repressistan citizens much stronger
protections against being singled out of the larger population
of Repressistan citizens who successfully connects to this forum
by whatever means.

In this scenario, Dissent is synergistic with existing and new
point-to-point anonymity, unobservability, and unblockability schemes.
The point-to-point schemes ensure Repressistan citizens
{\em access} to an external forum guaranteeing freedom of speech,
while building that forum on Dissent
protects the actual {\em content} of their sensitive speech
from being linked to their real-world identities
even via strong traffic analysis attacks.
Dissent effectively guarantees that the RATS cannot identify
and selectively punish
the specific group member(s) responsible,
and one hopes the group is too large --
and/or the RATS ``not totalitarian enough'' --
to punish {\em all} members of the group harshly (e.g., jail or worse)
purely on the basis of a few group members' actions.
Furthermore, even if the RATS infiltrates the forum
and tries to spam or otherwise disrupt the operation of the forum,
Dissent guarantees that non-compromised group members
can {\em detect} the source of RATS' blocking or other disruption,
and re-form new Dissent groups among members who can still communicate
while excluding any identified disruptors.

### Use Case Procedural Details ###

XXX to be written

## Use Case 3: The ``Extremists At Home'' Scenario ##

### Basic Scenario ###

A reasonably large group of ordinary citizens
of a generally free country,
who wish to discuss or blog freely on politically sensitive topics,
on which certain positions are known sometimes
to draw illegal attacks from extremists.
While one might hope a free country's legal and law enforcement system,
together with normal online privacy protections,
should protect users legally wishing to discuss sensitive topics
or take legal but sensitive positions,
experience has shown that hacker groups representing extremists
can break into web sites or other online forums
and steal identifying personal information
useful by the fringe for intimidation and/or physical attacks.
Running such forums as Dissent groups could offer users
stronger guarantees of protection against such threats.
A few specific examples might include:

*	A large online support group for victims of domestic abuse,
	whose members risk further violent abuse
	if their (potentially technically savvy) abusers
	are able to re-identify and trace them online.
*	A large online forum dedicated to discussing abortion,
	{\em a few} members of whom are doctors providing abortions
	who wish to relate their experiences anonymously
	without risking their real identities or home addresses
	becoming known to hacker groups
	representing anti-abortion extremist groups.
*	A large online forum dedicated to discussing
	all aspects of the illegal drug trade,
	{\em a few} members of whom are law enforcement officers,
	and/or former witnesses under government protection,
	who wish to avoid the risk of their real identities
	becoming known to drug traffickers still at large
	and having access to substantial ``hacking resources.''
*	A large online forum dedicated to discussing Islam,
	{\em a few} members of whom are political cartoonists
	who occasionally wish to post cartoons
	depicting the prophet Mohammed,
	without risking reprisals from Islamic militants.

### Use Case Procedural Details ###

XXX to be written

