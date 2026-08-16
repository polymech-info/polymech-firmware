# Extruder - Serial Protocol

## Protocols

### Serial

#### Request

Since it's serial, we receive for each command a reply matching an issue id as well a payload with the requested data or command replies.

**Command construction** :

Send Data/Command Syntax : `` `ID ; VERB ; FLAGS ; VERSION ; PAYLOAD` ``

**ID** : queued/issued command id, used to confirm command on sender side

**VERB** : COMMAND,STATUS,DATA

**FLAGS** :
```c++
    enum MessageFlags
    {
        NEW = 1 << 1,           // set on target when inbound
        // set on target
        PROCESSING = 1 << 2,
        // set on target when inbound
        PROCESSED = 1 << 3,
        // set on host, turn on debugging through the entire processing chain
        DEBUG = 1 << 4,
        RECEIPT = 1 << 5        // set on host, this will return the new state
    };

```

**PAYLOAD**: String, this string depends on the verb (see *ECALLS*).

Payloads is the actual call, in the form of Class:Function:Parameter(short),

### Example

```
    "1;2;64;1;VFD:fwd:1"
```

```c++
enum ECALLS
{
    // global function
    EC_COMMAND = 1,
    // addon method
    EC_METHOD = 2,
    // external function
    EC_FUNC = 3,
    // user space
    EC_USER = 10

};
```

**Format for Verb EC_METHOD** : Addon-Class-Name:Addon-Class-MemberFunction-Name:Argument

#### Response

Response construction via delimitter : 10|x0A - line by line

Response syntax : `` `ID ; STATUS ;  PAYLOAD` ``

**ID**: queued/issued command id, used to confirm command on sender side

**STATUS** : Error code, OK=0, SERVERITY Mask (syslog)

**PAYLOAD** : String - this string contains all enabled module states. The payload depends upon the sent query type (see **ECALLS**).

**Format for Verb EC_METHOD**, ie: ``` `Power:off:1` ``` = NEW_VALUE (the new state)

------------------------------------------------------------
