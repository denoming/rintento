
# Class diagrams

```plantuml
@startuml
!theme vibrant

class RecognitionObserver {
    +ready(): bool
    +wait()
    +get(error&): Utterances
    +cancel()
}

WitRecognitionObserver <|-- RecognitionObserver

class WitMessageRecognition {
    {static} create(context, executor)
    +run(host, port, auth, message)
}

class WitSpeechRecognition {
    {static} create(context, executor)
    +run(host, port, auth, path)
}

class WitRecognition {
    +cancel()
    +onData(slot): connection <signal>
    +onError(slot): connection <signal>
    +onSuccess(slot): connection <signal>
}

WitMessageRecognition <|-- WitRecognition
WitSpeechRecognition <|-- WitRecognition

class RecognitionConnection
class IntentRecognizeSession

IntentRecognizeMessageSession <|-- IntentRecognizeSession
IntentRecognizeSpeechSession <|-- IntentRecognizeSession

class RecognitionServer {
    +start()
    +stop()
}

@enduml
```