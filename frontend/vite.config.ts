import { defineConfig } from 'vite'
import react from '@vitejs/plugin-react'
import path from 'path'
import dotenv from 'dotenv'

export default defineConfig(() => {
  // 从项目根目录加载 .env 文件
  const envPath = path.resolve(__dirname, '..', '.env')
  const env = dotenv.config({ path: envPath }).parsed || {}

  // 从环境变量获取配置，不存在则抛出错误
  const frontendPort = parseInt(env.FRONTEND_PORT, 10)
  const host = env.HOST
  const apiPort = parseInt(env.API_PORT, 10)
  const backendPort = parseInt(env.BACKEND_PORT, 10)

  // 验证必需的环境变量
  if (!host) {
    throw new Error('HOST 环境变量未定义')
  }
  if (isNaN(frontendPort)) {
    throw new Error('FRONTEND_PORT 环境变量未定义或不是有效数字')
  }
  if (isNaN(apiPort)) {
    throw new Error('API_PORT 环境变量未定义或不是有效数字')
  }
  if (isNaN(backendPort)) {
    throw new Error('BACKEND_PORT 环境变量未定义或不是有效数字')
  }

  return {
    plugins: [react()],
    define: {
      'import.meta.env.VITE_BACKEND_URL': JSON.stringify(`http://${host}:${backendPort}`),
      'import.meta.env.VITE_BACKEND_HOST': JSON.stringify(host),
      'import.meta.env.VITE_BACKEND_PORT': JSON.stringify(backendPort.toString()),
      'import.meta.env.VITE_API_URL': JSON.stringify(`http://${host}:${apiPort}`),
      'import.meta.env.VITE_API_HOST': JSON.stringify(host),
      'import.meta.env.VITE_API_PORT': JSON.stringify(apiPort.toString()),
    },
    server: {
      port: frontendPort,
      proxy: {
        '/api': {
          target: `http://${host}:${apiPort}`,
          changeOrigin: true,
        },
        '/images': {
          target: `http://${host}:${apiPort}`,
          changeOrigin: true,
        },
      },
    },
  }
})