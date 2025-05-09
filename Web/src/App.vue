<script setup lang="ts">
import PrizeBox from "./lib/prizebox";

import { ref } from "vue";

enum MessageType {
  KEY_PRESS = 0x01,
  SET_LIGHT_STATE = 0x02,
  TRIGGER_UNLOCK = 0x03,
  TRIGGER_BUZZER = 0x04,
}

let prizeBox = new PrizeBox();

prizeBox.onMessage((data) => {
  const type = data[2];
  const index = data[3];
  const value = data[4];

  if (type == 0x01) active.value[index] = value;

  if (type == 0x02) {
    if (value == 1) {
      triggerState.value[index] = 1;
    } else {
      triggerState.value[index] = 0;
    }
  }
});

prizeBox
  .connect()
  .then(() => {
    console.log("Connected to MIDI device");
  })
  .catch((err) => {
    console.error("Failed to connect to MIDI device", err);
  });

const triggers = ["SUCCESS", "FAILURE", "STANDBY"];
const triggerState = ref([0, 0, 0]);

const keys = [1, 2, 3, 4, 5, 6, 7, 8, 9, "Cancel", 0, "Enter"];
const active = ref([0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]);
</script>

<template>
  <div class="flex h-screen">
    <div class="w-1/4 bg-gray-100 p-4">
      <button
        class="trigger"
        @click="
          prizeBox.send([0xf0, 0x7d, MessageType.SET_LIGHT_STATE, 0, 1, 0xf7])
        "
      >
        SUCCESS ON
      </button>
      <button
        class="trigger"
        @click="
          prizeBox.send([0xf0, 0x7d, MessageType.SET_LIGHT_STATE, 0, 0, 0xf7])
        "
      >
        SUCCESS OFF
      </button>
      <button
        class="trigger"
        @click="
          prizeBox.send([0xf0, 0x7d, MessageType.SET_LIGHT_STATE, 1, 1, 0xf7])
        "
      >
        FAILURE ON
      </button>
      <button
        class="trigger"
        @click="
          prizeBox.send([0xf0, 0x7d, MessageType.SET_LIGHT_STATE, 1, 0, 0xf7])
        "
      >
        FAILURE OFF
      </button>
      <button
        class="trigger"
        @click="
          prizeBox.send([0xf0, 0x7d, MessageType.SET_LIGHT_STATE, 2, 1, 0xf7])
        "
      >
        STANDBY ON
      </button>
      <button
        class="trigger"
        @click="
          prizeBox.send([0xf0, 0x7d, MessageType.SET_LIGHT_STATE, 2, 0, 0xf7])
        "
      >
        STANDBY OFF
      </button>

      <button
        class="trigger"
        @click="prizeBox.send([0xf0, 0x7d, MessageType.TRIGGER_UNLOCK, 0xf7])"
      >
        UNLOCK
      </button>

      <button
        class="trigger"
        @click="prizeBox.send([0xf0, 0x7d, MessageType.TRIGGER_BUZZER, 0xf7])"
      >
        BEEP
      </button>
    </div>
    <div class="flex-1 bg-white">
      <div class="grid grid-cols-4 gap-4 p-4 max-w-sm mx-auto">
        <button
          class="bg-green-800 text-white rounded-2xl text-xs py-6 px-2 shadow-md active:bg-green-500"
          :class="[triggerState[0] ? 'bg-gray-300' : 'bg-blue-500']"
        >
          SUCCESS
        </button>
        <button
          class="bg-red-800 text-white rounded-2xl text-xs py-6 px-2 shadow-md active:bg-red-500"
        >
          FAILURE
        </button>
        <button
          class="bg-yellow-800 text-white rounded-2xl text-xs py-6 px-2 shadow-md active:bg-yellow-500"
        >
          STAND BY
        </button>
        <button
          class="bg-blue-800 text-white rounded-2xl text-xs py-6 px-2 shadow-md active:bg-blue-500"
        >
          UNLOCK
        </button>
      </div>
      <div class="grid grid-cols-3 gap-4 p-4 max-w-sm mx-auto">
        <button
          v-for="(key, index) in keys"
          :key="key"
          class="bg-gray-800 text-white rounded-2xl text-xl py-6 px-2 shadow-md"
          :class="[active[index] == 1 ? 'bg-pink-500' : 'bg-gray-800']"
        >
          {{ key }}
        </button>
      </div>
    </div>
  </div>
</template>

<style scoped></style>
