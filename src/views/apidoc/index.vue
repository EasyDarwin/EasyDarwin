<script setup></script>

<template>
  <a-card class="apidoc-iframe">
    <iframe
      src="/apidoc.html">
  </iframe>
  </a-card>
</template>

<style  lang="less">
.apidoc-iframe {
  width: 100%;
  height: 100%;

  .ant-card-body{
    width: 100%;
    height: 100%;
    iframe {
      border: 0;
      width: 100%;
      height: 100%;
    }
  }
}

</style>
