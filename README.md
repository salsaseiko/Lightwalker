# Lightwalker

## Descrizione
Il progetto consiste in un robottino (al momento un prototipo di cartone) che si muove appena percepisce una fonte di luce.
È costruito nel seguente modo:
- il "corpo" è una scatola di cartone contenente l'alimentazione e una breadboard per collegare tutti i componenti;
- le "gambe" sono costituite da due stecchetti collegati a due servo motori;
- ogni stecchetto ha due "piedi" con cui si muove;
- presenta tre "occhi", due fotoresistenze ed un modulo fotoresistivo con uscita digitale.

Puntando la fonte direttamente davanti agli "occhi" procederà a muoversi in avanti, spostandola verso l'occhio destro e sinistro procederà a virare rispettivamente a destra e sinistra.
Appena non percepisce più la luce si fermerà.

## Componenti hardware
- Esp32
- Breadboard
- Alimentazione (6.0V, 2800mAh)
- 2 fotoresistenze
- un modulo fotoresistivo con uscita digitale
- 2 resistenze da 220 Ohm
- 2 servo motori SG-90

## Schema

## Scelte progettuali
### Calcolo della direzione
Il robot procede a muoversi appena percepisce una fonte luminosa dall'occhio costituito dal modulo LDR con uscita digitale (0 = percepisce luce, 1 = buio).
Decide in che direzione muoversi in base alla differenza di luce che percepisce tra i due occhi composti dalle LDR.
- Se tale differenza, in valore assoluto, è sotto un determinato THRESHOLD, procede dritto.
- Se tale differenza supera il THRESHOLD in negativo vira a sinistra, a destra altrimenti.

### Media mobile
In quanto le LDR erano molto sensibili al minimo cambiamento della luce (passando da 0 al valore massimo al minimo avvicinamento della torcia), per rendere il passaggio più graduale ho utilizzato una tecnica a livello software chiamata "media mobile" (calcolata sia per la LDR a sinistra che a destra con una specifica funzione). 
Come risultato di quanto letto dalla fotoresistanza non si prende l’ultimo campione ma la media degli ultimi N campioni. Scegliendo un N alto si ha un passaggio più graduale, ma per questo robottino ho scelto un N più basso per renderlo abbastanza reattivo. 

### Uso di un interrupt
Per far frenare il robot appena viene tolta la fonte di luce ho utilizzato un interrupt collegato al pin con il modulo LDR con uscita digitale. Appena accade una situazione di RISING (da 0 luce ad 1 buio), la ISR setta la variabile stop a true ed il robot si ferma.

## Considerazioni
### Possibili alternative
Come alternativa alla soluzione software della media mobile si potrebbero usare dei condensatori...

