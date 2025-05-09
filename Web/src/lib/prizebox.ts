export default class PrizeBox {
  private callback: (data: Uint8Array) => void = () => {};
  private output: MIDIOutput | null = null;

  async connect(): Promise<void> {
    const midiAccess = await navigator.requestMIDIAccess({ sysex: true });
    const outputs = Array.from(midiAccess.outputs.values());
    const inputs = Array.from(midiAccess.inputs.values());

    if (!outputs.length || !inputs.length) {
      throw new Error("No MIDI ports found");
    }

    this.output = outputs[0];

    inputs[0].onmidimessage = (e: WebMidi.MIDIMessageEvent) => {
      console.log("Received", e.data);
      this.callback(new Uint8Array(e.data));
    };

    outputs.forEach((o) => console.log("Output:", o.name));
    inputs.forEach((i) => console.log("Input:", i.name));
  }

  send(bytes: number[] | Uint8Array): void {
    if (!this.output) throw new Error("Not connected");
    console.log("Sending", bytes);
    this.output.send(bytes);
  }

  onMessage(cb: (data: Uint8Array) => void): void {
    this.callback = cb;
  }
}
