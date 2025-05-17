<script setup lang="ts">
import { PrizeBox, MessageType, BoardIdentifier } from "./lib/prizebox";

import type { Ref } from "vue";
import { ref } from "vue";

const keys = [1, 2, 3, 4, 5, 6, 7, 8, 9, "Cancel", 0, "Enter"];
const keyState = ref<number[]>([]);

const keyMap = [
  MessageType.KEY_PRESS_1,
  MessageType.KEY_PRESS_2,
  MessageType.KEY_PRESS_3,
  MessageType.KEY_PRESS_4,
  MessageType.KEY_PRESS_5,
  MessageType.KEY_PRESS_6,
  MessageType.KEY_PRESS_7,
  MessageType.KEY_PRESS_8,
  MessageType.KEY_PRESS_9,
  MessageType.KEY_PRESS_CANCEL,
  MessageType.KEY_PRESS_0,
  MessageType.KEY_PRESS_ENTER,
];

const prizeBox = new PrizeBox();
const board: Ref<BoardIdentifier> = ref(BoardIdentifier.CODETV_BOARD_0);

prizeBox.onMessage((id: BoardIdentifier, data: Uint8Array) => {
  console.log("Board:", id, "Message:", data);

  const type = data[2];
  const value = data[3];

  keyState.value[type] = value;

  if (type == MessageType.TRIGGER_UNLOCK) {
    setTimeout(() => {
      keyState.value[type] = 0;
    }, 100);
  }
});

prizeBox.onKey((id: BoardIdentifier, key: number) => {
  console.log("Board:", id, "Key:", key);
});

prizeBox.onCancel((id: BoardIdentifier) => {
  console.log("Board:", id, "Key:", "Cancel");
});

prizeBox.onEnter((id: BoardIdentifier) => {
  console.log("Board:", id, "Key:", "Enter");
});

prizeBox
  .connect()
  .then(() => {
    console.log("Connected to MIDI device");
  })
  .catch((err) => {
    console.error("Failed to connect to MIDI device", err);
  });
</script>

<template>
  <div class="flex h-screen">
    <div class="w-1/4 bg-gray-100 p-4">
      <select v-model="board" class="border rounded py-2 w-full mb-2">
        <option :value="BoardIdentifier.CODETV_BOARD_0">CODETV_BOARD_0</option>
        <option :value="BoardIdentifier.CODETV_BOARD_1">CODETV_BOARD_1</option>
        <option :value="BoardIdentifier.CODETV_BOARD_2">CODETV_BOARD_2</option>
        <option :value="BoardIdentifier.CODETV_BOARD_3">CODETV_BOARD_3</option>
      </select>

      <button class="trigger" @click="prizeBox.setFailureOn(board)">
        FAILURE ON
      </button>
      <button class="trigger" @click="prizeBox.setFailureOff(board)">
        FAILURE OFF
      </button>

      <button class="trigger" @click="prizeBox.setSuccessOn(board)">
        SUCCESS ON
      </button>
      <button class="trigger" @click="prizeBox.setSuccessOff(board)">
        SUCCESS OFF
      </button>

      <button class="trigger" @click="prizeBox.setStandByOn(board)">
        STANDBY ON
      </button>
      <button class="trigger" @click="prizeBox.setStandByOff(board)">
        STANDBY OFF
      </button>
      <button class="trigger" @click="prizeBox.triggerUnlock(board)">
        UNLOCK
      </button>

      <button class="trigger" @click="prizeBox.triggerBuzzer(board)">
        BEEP
      </button>

      <button class="trigger" @click="prizeBox.triggerReset(board)">
        RESET
      </button>
    </div>
    <div class="flex-1 bg-white">
      <div class="grid grid-cols-4 gap-4 p-4 max-w-sm mx-auto">
        <button
          class="text-white rounded-2xl text-xs py-6 px-2 shadow-md"
          :class="[
            keyState[MessageType.SET_FAILURE] == 1
              ? 'bg-red-400'
              : 'bg-red-800',
          ]"
        >
          FAILURE
        </button>
        <button
          class="text-white rounded-2xl text-xs py-6 px-2 shadow-md"
          :class="[
            keyState[MessageType.SET_SUCCESS] == 1
              ? 'bg-green-400'
              : 'bg-green-800',
          ]"
        >
          SUCCESS
        </button>
        <button
          class="text-white rounded-2xl text-xs py-6 px-2 shadow-md"
          :class="[
            keyState[MessageType.SET_STANDBY] == 1
              ? 'bg-blue-400'
              : 'bg-blue-800',
          ]"
        >
          STAND BY
        </button>
        <button
          class="text-white rounded-2xl text-xs py-6 px-2 shadow-md"
          :class="[
            keyState[MessageType.TRIGGER_UNLOCK] == 1
              ? 'bg-yellow-400'
              : 'bg-yellow-800',
          ]"
        >
          UNLOCK
        </button>
      </div>
      <div class="grid grid-cols-3 gap-4 p-4 max-w-sm mx-auto">
        <button
          v-for="(key, index) in keys"
          :key="key"
          class="bg-gray-800 text-white rounded-2xl text-xl py-6 px-2 shadow-md"
          :class="[
            keyState[keyMap[index]] == 1 ? 'bg-pink-500' : 'bg-gray-800',
          ]"
        >
          {{ key }}
        </button>
      </div>
    </div>
  </div>
</template>

<style scoped></style>
