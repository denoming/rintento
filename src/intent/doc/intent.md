# Introduction

Intent component is a main component of the `Rintento` service. This component includes server, recognition handlers and
backend clients.

# Purpose

The purpose is recognizing known intent by text message or audio speech and perform action upon client request.

# Use Cases

* Intent recognizing

```plantuml
@startuml

actor Client
actor Server

usecase "Recognize intent by text message input" as UC1
usecase "Recognize intent by audio speech" as UC2

Client --> UC1
Client --> UC2

UC1 --> Server
UC2 --> Server

@enduml
```

# Component Structure

## Software Units

| Name                       | Description                                                         |
|----------------------------|---------------------------------------------------------------------|
| RecognitionSession         | The dispatcher to handle recognition sent to backend                |
| RecognitionHandler         | Represents base class for recognition handler                       |
| RecognitionMessageHandler  | The message recognition handler to handle particular client request |
| RecognitionSpeechHandler   | The speech recognition handler to handle particular client request  |
| RecognitionTerminalHandler | The terminal (empty) handler                                        |
| RecognitionServer          | The server to accept client connections                             |
| SpeechDataBuffer           | The data buffer to receive and store client audio data              |
| WitIntentParser            | The wit.ai intent parser                                            |
| WitMessageRecognition      | The wit.ai message recognition request to backend                   |
| WitSpeechRecognition       | The wit.ai speech recognition request to backend                    |
| WitRecognition             | Represents base class for recognition request to backend            |
| WitRecognitionFactory      | The factory to create recognition request to backend                |
| IntentSubsystem            | The subsystem to handle lifetime of service software units          |

## Class Diagrams

* Subsystem

```plantuml
@startuml
abstract class Subsystem

interface IConfig {
    +proxyServerPort(): std::uint16_t
    +proxyServerThreads(): std::size_t
    +recognizeServerHost(): std::string_view
    +recognizeServerPort(): std::string_view
    +recognizeServerAuth(): std::string_view
    +recognizeThreads(): std::size_t
}

Subsystem <|-- IntentSubsystem
IConfig <|- Config

IntentSubsystem o-- Worker : 2
IntentSubsystem o-- Config
IntentSubsystem o-- RecognitionServer
IntentSubsystem o-- WitRecognitionFactory
@enduml
```

* Recognition server

```plantuml
@startuml
set namespaceSeparator ::

class RecognitionServer {
    {static} create(executor, factory)
    +listen(port): bool
    +listen(endpoint): bool
    +shutdown()
}

RecognitionServer o-- RecognitionSession : 0..*
RecognitionServer o-- WitRecognitionFactory

class RecognitionHandler { 
    +onComplete(callback)
    +setNext(handler)
    +handle()
    #submit(result)
    #submit(error)
    #sendResponse(result)
    #sendResponse(error)
}

RecognitionHandler <|-- RecognitionMessageHandler
RecognitionHandler <|-- RecognitionSpeechHandler
RecognitionHandler <|-- RecognitionTerminalHandler

RecognitionHandler o-- RecognitionHandler

RecognitionMessageHandler o-- WitRecognitionFactory
RecognitionMessageHandler o-- WitMessageRecognition

RecognitionSpeechHandler o-- WitRecognitionFactory
RecognitionSpeechHandler o-- WitSpeechRecognition
RecognitionSpeechHandler o- SpeechDataBuffer

RecognitionSession o-- WitRecognitionFactory
RecognitionSession o-- RecognitionHandler
@enduml
```

* The wit.ai backend classes

```plantuml
@startuml
class WitRecognition {
    +needData(): bool
    +done(): bool
    +wait()
    +onComplete(callback)
    +onData(callback)
    #needData()
    #setResult(value)
    #setError(value)
}

Cancellable <|-- WitRecognition
WitRecognition <|-- WitMessageRecognition 
WitRecognition <|-- WitSpeechRecognition

class WitRecognitionFactory {
    +message()
    +speech()
}

@enduml
```

## Sequence Diagram

* Handle client recognition requests

```plantuml
@startuml

participant RecognitionServer
participant RecognitionSession

-> RecognitionServer : onAcceptDone(error, socket)
RecognitionServer -> RecognitionServer : spawnSession(socket)
create RecognitionSession
RecognitionServer -> RecognitionSession : create(socket, factory) 
activate RecognitionServer
RecognitionServer -> RecognitionSession : run(id, socket, factory)
activate RecognitionSession
RecognitionServer -> RecognitionServer : accept()
note right: waiting for new connections
...reading header data from socket...
-> RecognitionSession : onReadHeaderDone(errorCode, bytes)
alt if EoS or error
    RecognitionSession -> RecognitionSession : finalize()
    RecognitionSession -> RecognitionServer : onSessionComplete(id)
    opt ready to shutdown
        RecognitionServer -> RecognitionServer : notifyShutdownReady()
    end
    deactivate RecognitionServer
end
RecognitionSession -> RecognitionSession : getHandler()
create RecognitionHandler
RecognitionSession -> RecognitionHandler : create()
activate RecognitionHandler
RecognitionSession -> RecognitionHandler : handle()
...handle message or speech recognition...
RecognitionHandler -> RecognitionSession : onComplete(utterances, error)
deactivate RecognitionHandler
RecognitionSession -> RecognitionSession : doReadHeader()
note right: read next header (if present)
deactivate ActionPerformer
deactivate RecognitionSession

@enduml
```

* Handle message recognition request

```plantuml
@startuml

participant RecognitionSession
participant RecognitionMessageHandler
participant WitRecognitionFactory
participant WitMessageRecognition

activate RecognitionSession
RecognitionSession -> RecognitionMessageHandler : handle()
activate RecognitionMessageHandler
break if not message recognition
    RecognitionMessageHandler -> RecognitionMessageHandler : handle()
    note right: move to the next handler in the chain
end
break peek message has failed
    RecognitionMessageHandler -> RecognitionMessageHandler : onRecognitionError(<bad message>)
end
RecognitionMessageHandler -> RecognitionMessageHandler : createRecognition()
RecognitionMessageHandler -> WitRecognitionFactory : message()
activate WitRecognitionFactory
create WitMessageRecognition
WitRecognitionFactory -> WitMessageRecognition : create(config, context, executor)
deactivate WitRecognitionFactory
RecognitionMessageHandler ->> WitMessageRecognition : run()

activate WitMessageRecognition
RecognitionSession <- RecognitionMessageHandler : handle(buffer, parser)
...
WitMessageRecognition -> RecognitionMessageHandler : onRecognitionData()
RecognitionMessageHandler -> WitMessageRecognition : feed(message)
...
alt recognition on backend was successful
    WitMessageRecognition -> RecognitionMessageHandler : onRecognitionSuccess(result)
    RecognitionMessageHandler -> RecognitionMessageHandler : sendResponse(result)
    RecognitionMessageHandler -> RecognitionMessageHandler : submit(result)
    RecognitionSession <- RecognitionMessageHandler : onComplete(utterances, <empty>)
else
    WitMessageRecognition -> RecognitionMessageHandler : onRecognitionError(error)
    deactivate WitMessageRecognition
    RecognitionMessageHandler -> RecognitionMessageHandler : sendResponse(error)
    RecognitionMessageHandler -> RecognitionMessageHandler : submit(error)
    RecognitionSession <- RecognitionMessageHandler : onComplete(<empty>, error)
    deactivate RecognitionSession
end
deactivate RecognitionMessageHandler

@enduml
```

* Handle client speech recognition request

```plantuml
@startuml

participant RecognitionSession
participant RecognitionSpeechHandler
participant WitRecognitionFactory
participant WitSpeechRecognition

activate RecognitionSession
RecognitionSession -> RecognitionSpeechHandler : handle()
activate RecognitionSpeechHandler
break if not speech recognition
    RecognitionSpeechHandler -> RecognitionSpeechHandler : handle()
    note right: move to the next handler in the chain
end
break if http::field::expect is not '100-continue' 
    RecognitionSpeechHandler -> RecognitionSpeechHandler : onRecognitionError(<operation not supported>)
end
RecognitionSpeechHandler -> RecognitionSpeechHandler : createRecognition()
RecognitionSpeechHandler -> WitRecognitionFactory : speech()
activate WitRecognitionFactory
create WitSpeechRecognition
WitRecognitionFactory -> WitSpeechRecognition : create(config, context, executor)
deactivate WitRecognitionFactory
RecognitionSpeechHandler ->> WitSpeechRecognition : run()
activate WitSpeechRecognition
RecognitionSpeechHandler -> RecognitionSpeechHandler : handleSpeechData(buffer, parser)
loop while audio data is present
    RecognitionSpeechHandler -> WitSpeechRecognition : needData()
    opt audio data is needed
        RecognitionSpeechHandler -> RecognitionSpeechHandler : onRecognitionData()
        opt minimum data available
            RecognitionSpeechHandler -> SpeechDataBuffer : extract()
            RecognitionSpeechHandler -> WitSpeechRecognition : feed(buffer)
        end
        opt is data buffer finalized
            RecognitionSpeechHandler -> WitSpeechRecognition : finalize()
        end        
    end
end
...
alt recognition on backend was successful
    WitSpeechRecognition -> RecognitionSpeechHandler : onRecognitionSuccess(result)
    RecognitionSpeechHandler -> RecognitionSpeechHandler : sendResponse(result)
    RecognitionSpeechHandler -> RecognitionSpeechHandler : submit(result)
    RecognitionSession <- RecognitionSpeechHandler : onComplete(utterances, <empty>)
else
    WitSpeechRecognition -> RecognitionSpeechHandler : onRecognitionError(error)
    deactivate WitSpeechRecognition
    RecognitionSpeechHandler -> RecognitionSpeechHandler : sendResponse(error)
    RecognitionSpeechHandler -> RecognitionSpeechHandler : submit(error)
    RecognitionSession <- RecognitionSpeechHandler : onComplete(<empty>, error)
    deactivate RecognitionSession
end
deactivate RecognitionSpeechHandler


@enduml
```

* Handle message recognition request to backend

```plantuml
@startuml

participant RecognitionMessageHandler
participant WitMessageRecognition
participant Stream
participant Resolver

activate RecognitionMessageHandler
RecognitionMessageHandler -> WitMessageRecognition : run()
activate WitMessageRecognition
break invalid backend config to connect
    WitMessageRecognition -> WitMessageRecognition : setError(<invalid argument>)
end
WitMessageRecognition -> WitMessageRecognition : run(host, port, auth)
WitMessageRecognition ->> Resolver : resolve(host, port)
activate Resolver
...
Resolver -> WitMessageRecognition : onResolveDone(error, result)
deactivate Resolver
break was cancelled
    WitMessageRecognition -> WitMessageRecognition : setError(<operation canceled>)
end
WitMessageRecognition ->> Stream : connect(result)
activate Stream
...
Stream -> WitMessageRecognition : onConnectDone(error, endpoint)
break was cancelled
    WitMessageRecognition -> WitMessageRecognition : setError(<operation canceled>)
end
WitMessageRecognition ->> Stream : handshake(error, endpoint)
...
Stream -> WitMessageRecognition : onHandshakeDone(error)
alt was cancelled
    WitMessageRecognition -> WitMessageRecognition : setError(<operation canceled>)
else
    WitMessageRecognition -> RecognitionMessageHandler : needData()
end
...
RecognitionMessageHandler -> WitMessageRecognition : feed(message)
deactivate RecognitionMessageHandler
WitMessageRecognition ->> Stream : write(target)
...
Stream -> WitMessageRecognition : onWriteDone(error, bytesTransferred)
break was cancelled
    WitMessageRecognition -> WitMessageRecognition : setError(<operation canceled>)
end
WitMessageRecognition ->> Stream : read()
...
Stream -> WitMessageRecognition : onReadDone(error, bytesTransferred)
WitMessageRecognition -> WitMessageRecognition : setResult(<json>)
break was cancelled
    WitMessageRecognition -> WitMessageRecognition : setError(<operation canceled>)
end
WitMessageRecognition -> Stream : shutdown()
...
Stream -> WitMessageRecognition : onShutdownDone(error)
deactivate Stream
deactivate WitMessageRecognition

@enduml
```

* Handle speech recognition request to backend

```plantuml
@startuml

participant RecognitionSpeechHandler
participant WitSpeechRecognition
participant Stream
participant Resolver

activate RecognitionSpeechHandler
RecognitionSpeechHandler -> WitSpeechRecognition : run()
activate WitSpeechRecognition
break invalid backend config to connect
    WitSpeechRecognition -> WitSpeechRecognition : setError(<invalid argument>)
end
WitSpeechRecognition -> WitSpeechRecognition : run(host, port, auth)
WitSpeechRecognition ->> Resolver : resolve(host, port)
activate Resolver
...
Resolver -> WitSpeechRecognition : onResolveDone(error, result)
deactivate Resolver
break was cancelled
    WitSpeechRecognition -> WitSpeechRecognition : setError(<operation canceled>)
end
WitSpeechRecognition ->> Stream : connect(result)
activate Stream
...
Stream -> WitSpeechRecognition : onConnectDone(error, endpoint)
break was cancelled
    WitSpeechRecognition -> WitSpeechRecognition : setError(<operation canceled>)
end
WitSpeechRecognition ->> WitSpeechRecognition : handshake(error, endpoint)
...
Stream -> WitSpeechRecognition : onHandshakeDone(error)
break was cancelled
    WitSpeechRecognition -> WitSpeechRecognition : setError(<operation canceled>)
end
WitSpeechRecognition -> WitSpeechRecognition : readContinue()
...
Stream -> WitSpeechRecognition : onReadContinueDone(error, bytesTransferred)
break invalid HTTP status
   WitSpeechRecognition -> WitSpeechRecognition : setError(<illegal byte sequence>)
end
alt was cancelled
    WitSpeechRecognition -> WitSpeechRecognition : setError(<operation canceled>)
else 
    WitSpeechRecognition -> RecognitionSpeechHandler : needData(true)
end
...
RecognitionSpeechHandler -> WitSpeechRecognition : feed(buffer)
WitSpeechRecognition -> RecognitionSpeechHandler : needData(false)
WitSpeechRecognition ->> WitSpeechRecognition : writeNextChunk(buffer)
...
Stream -> WitSpeechRecognition : onWriteNextChunkDone(error, bytesTransferred)
alt was cancelled
    WitSpeechRecognition -> WitSpeechRecognition : setError(<operation canceled>)
else 
    WitSpeechRecognition -> RecognitionSpeechHandler : needData(true)
end
...repeat writing chunks until run out audio data ...
RecognitionSpeechHandler -> WitSpeechRecognition : finalize()
WitSpeechRecognition ->> WitSpeechRecognition : writeLastChunk()
...
Stream -> WitSpeechRecognition : onWriteLastChunkDone(error, bytesTransferred)
break was cancelled
    WitSpeechRecognition -> WitSpeechRecognition : setError(<operation canceled>)
end
WitSpeechRecognition ->> WitSpeechRecognition : read()
...
Stream -> WitSpeechRecognition : onReadDone(error, bytesTransferred)
WitSpeechRecognition -> WitSpeechRecognition : setResult(<json>)
break was cancelled
    WitSpeechRecognition -> WitSpeechRecognition : setError(<operation canceled>)
end
WitSpeechRecognition -> WitSpeechRecognition : shutdown()
...
Stream -> WitSpeechRecognition : onShutdownDone(error)
deactivate Stream
deactivate WitSpeechRecognition

@enduml
```