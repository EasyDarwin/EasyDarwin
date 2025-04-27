<h1 align="center" style="margin: 30px 0 30px; font-weight: bold;">Web_src Source Management </h1>

##  Overview

- This repository is a backend management system template built with the frontend tech stack: [Vue3](https://v3.cn.vuejs.org), [Vite](https://cn.vitejs.dev), [Ant](https://www.antdv.com/docs/vue/introduce-cn), and [Pinia](https://pinia.vuejs.org/zh/introduction.html).

## Setup

```bash
# Clone the project
git clone https://github.com/EasyDarwin/EasyDarwin.git

# Navigate to the project directory
cd EasyDarwin/web_src

# Node environment version: v18.19.0

# Install dependencies
npm install

# Start the development server
npm run dev

# Build the production environment
npm run build

# Build the test environment
npm run build

# Build the production environment
npm run build

# Frontend access URL: http://localhost:3001
```

## Structure

<pre>
├─public            # Static Resource Files
├─src               # Source Code Directory
│  ├─api            # API Request Related Files
│  ├─assets         # Static Assets
│  ├─components     # Reusable Vue Components
│  ├─layouts        # Layout Components
│  ├─plugins        # Plugin Configuration
│  ├─router         # Router Configuration
│  ├─settings       # Project Settings
│  ├─store          # Vuex State Management
│  ├─styles         # Style Files
│  ├─utils          # Utility Functions
│  └─views          # Page Views
├─.editorconfig     # Code Formatting Configuration
├─.env.development  # Development Environment Variables
├─.prettierrc.json  # Code Formatting Configuration
├─index.html        # Entry HTML File
├─jsconfig.json     # Project Configuration
├─package.json      # Project Dependencies
├─README.md         # Project Documentation
├─README_zh.md      # Project Documentation
├─vite.config.ts    # Vite Configuration Files
├─unocss.config.js  # Style Configuration Files
</pre>

## Built-in Features

1. **Login**: User login.
2. **Stream Pulling**: Add a stream pull in the live streaming service, then input the RTSP stream pulling address, and play it. Supported playback protocols(http-flv, http-hls, ws-flv, webrtc ).
3. **Stream Pushing**: Add a stream push in the live streaming service, edit it, copy the stream pushing address, paste it into the streaming platform, and play it. Supported playback protocols(http-flv, http-hls, ws-flv, webrtc).
4. **API Documentation**: View the API calls in the service.
5. **Version Information**: Record the current operating status of the service.
