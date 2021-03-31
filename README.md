# VOR

![vor](https://github.com/BM45/VOR/blob/main/pics4www/VOR.jpg)

VOR - ein experimenteller Radiokompass für gerichtete Drehfunkfeuer (VORs) für PC und Raspberry auf Basis eines Software-Defined-Radio

### Was sind VORs?

VORs sind Funkstationen die zur Navigation vornehmlich im UKW-Flugfunkbereich operieren. Im Gegensatz zu nicht-gerichteten Funkfeuern (u.a. NDBs auf Langwelle), die auf Seiten des Empfängers zum Beispiel mit einer sich drehenden Antenne angepeilt werden müssen, senden VORs die relative Richtung Sender<->Empänger "codiert" mit, können also mit einem einfachen Rundstrahler wie einer Teleskopantenne empfangen werden. Typische VOR-Sendestationen sehen so aus.

![vor2](https://github.com/BM45/VOR/blob/main/pics4www/vorreal.jpg)

VORs befinden sich in der Nähe wichtiger Flugrouten oder an Flughäfen. Über dieses VORs kann man seine Position auch ohne Satellitennavigationssysteme wie GPS oder GALILEO gut und ausreichend genau bestimmen. Ein Flugzeug kann sich also entlang dieser Flugfunkfeuer bis zu seinem Ziel hangeln. Aktive VORs findet man in Flugkarten eingezeichnet, diese gibt es für die eigene Region zum Beispiel unter skyvector.com . Hier ein Kartenausschnitt mit VORs um Frankfurt/Main.

![vor3](https://github.com/BM45/VOR/blob/main/pics4www/skyvector_com.jpg)

### Wie ist das Signal eines VOR aufgebaut?

Um die Funktionsweise eines VOR zu verstehen, schaut man sich am Besten das Signalspektrum einer solchen Station an.

![vor4](https://github.com/BM45/VOR/blob/main/pics4www/spektrumvor.jpg)

Das Sendesignal eines VOR besteht empfängerseitig aus zwei 30 Hz Signalen. Eines in Trägerlage und eines auf einem 9960Hz Unterträger. Ein 30Hz Signal erscheint im Empfänger immer dann, wenn das Richtfunkfeuer wie ein Leuchtturm in Richtung Norden (0°) "leuchten" würde. Das zweite 30-Hz Signal ist eine Richtungskomponente, die Empfängerseitig phasenschoben aufgenommen wird. Diese Richtungskomponente ist mit dem 0°-Nordsignal synchronisiert. Durch die Messung des Phasenversatzes beider 30Hz-Signale (die Phasenverschiebung kann ja nur zwischen 0° und 360° sein) erhält man die Richtungsinformation Sender<->Empfänger.

In dieser Abbildung beträgt der Phasenversatz beider 30Hz Signale 105°, das Flugzeug fliegt also auf dem 105°-Radial (Lichtkegel bei einem Leuchtturm).  
![vor5](https://upload.wikimedia.org/wikipedia/commons/d/d1/VOR_principle.gif)

Die Radialgenauigkeit solcher Funkfeuer wird je nach Bauart im Mittel mit 1-1.5° angegeben.Mit Hilfe zweier VOR-Stationen lässt sich somit ziemlich gut die eigene Position feststellen. Einige VOR besitzen noch eine DME-Einrichtung. DME bedeutet Distance Measuring Equipment, also eine Einrichtung zur Entfernungsmessung. Die DME-Unit arbeitet im Frequenzbereich um 1 GHz, ist also nicht Bestandteil des VOR-Signals. Eine DME-Einheit im Flugzeug sendet hierbei Pulse aus, die von der DME-Einheit am VOR wie eine Relaisstation zurückgestahlt werden. Durch die Signallaufzeit kann die DME-Einheit im Flugzeug somit die Schrägentfernung zur DME-Einheit am VOR bereichnen.  

Zusätzlich enthält das Signalkomplex im Bereich von 300-3300Hz ein Identificationssignal der VOR Station. Hier wird ununterbrochen die Kennung des VOR als Morsecode oder weitere Information als Sprache übertragen.
 
### Aufbau des Empfängers

Unser Empfänger arbeitet nach dem Software-Defined-Radio Prinzip. Er besteht aus dem HF-Frontend (zum Beispiel einen RTLSDR-USB Stick, Adalm Pluto, LimeSDR o.ä.) welches die Antennenspannung digitalisiert und diese Rohdaten an Software weiterleitet. Die ganze Signalverabeitung, also Demodulation und Darstellung des Signals erfolgt über Software.
Diese Software ist unterteilt in Signalverarbeitung (VORreceiver.grc -> VORreceiver.py), Signaldarstellung (Instrument.cxx) und ein paar Scripts die diese Komponenten miteinander verbinden und den Empfänger starten oder stoppen.

Die Signalverarbeitung wurde mit dem Werkzeug GNU-Radio Companion (GRC) erstellt. Die Teilbereiche des Empfängers will ich nachfolgend darstellen.

![vor6](https://github.com/BM45/VOR/blob/main/pics4www/VORreceiver_grc_sampling.jpg)

In diesem Teil wird die vom HF-Frontend abgetastete Antennespannung mit einer Samplerate von 2.048 MSPS eingelesen. Standardmäßig ist der HF-Frontendbaustein für einen RTLSDR USB Stick aktiv. Für andere Frontends oder im Vorfeld in einer Datei aufgezeichnete Hochfrequenz sind bereits Bausteine im Programm vorhanden, sie müssen vor der Verwendung nur per Mausklick aktiv geschaltet werden. Die abgetasteten Datenwerte werden um den Faktor 64 dezimiert (die geringe Bandbreite eines VOR-Signals lässt das zu und es ergibt sich ein Dezimierungsgewinn). Im Anschluss erfolgt die AM-Demodulation des Signals und die weitergabe in eine virtuelle Senke im Programm.

Im nächsten Schritt

![vor7](https://github.com/BM45/VOR/blob/main/pics4www/VORreceiver_grc_ref_var_sig.jpg)

erfolgt die Gewinnung beider 30Hz Signale, das eine in Trägerlage, das andere vom 9960Hz Unterträger. Das 0°-Nordsignal und die Richtungskomponente werden wiederum in Senken gegeben.

Die Auswertung der Phasenverschiebung beider Signale erfolgt in diesem Teil.

![vor8](https://github.com/BM45/VOR/blob/main/pics4www/VORreceiver_grc_diff.jpg)

Die auf 0-360° skalierte Phasenverschiebung wird dann als numerischer Wert pausenlos über eine UDP-Senke auf Port 1234 für weitere Softwarekomponenten (zum Beispiel dem Anzeigeninstrument) zur Verfügung gestellt.

![vor9](https://github.com/BM45/VOR/blob/main/pics4www/VORreceiver_grc_ident.jpg)

Über einen Bandpassfilter wird der Bereich von 300-3300Hz aus dem Basisband herausgefiltert und als Audio zur Standardsoundkarte des Rechners weitergeleitet. Über diese Audioinformation kann man herausfinden ob ein VOR-Signal überhaupt empfangen wird, die Anzeige des Instruments also gültig ist, und um welches VOR es sich handelt. 

Das gerade gezeigte GRC-Programm dient als Grundlage zur Erzeugung des Signalverarbeitungsskiptes, welches den Namen VORreceiver.py trägt. Bei Änderungen in der Signalverarbeitung (zum Beispiel Auswahl eines anderen HF-Frontendbausteins), muss dieses Skript durch GNU-Radio Companion neu erzeugt werden.

Eine weitere Softwarekomponente des Radiokompass ist unser Instrument, dessen Quellcode sich in Instrument.cxx befindet. Es nimmt die ermittelte Phasenverschiebung über Port 1234 vom GRC-Programm auf und zeichnet den eigentlichen Kompass. Die Darstellung der Richtungsangabe erfolgt dann wahlweise im VOM-VOR oder ZUM-VOR (inbound/outbound) Kurs. 

![vor10](https://github.com/BM45/VOR/blob/main/pics4www/Vor_inbound_outbound.jpg)





