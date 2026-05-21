/**
 * API 客户端 - 用于与后端 LangGraph 服务通信
 */
export class ApiClient {
  private baseUrl: string;
  private apiBaseUrl: string;

  constructor(baseUrl: string, apiUrl: string) {
    this.baseUrl = baseUrl;
    this.apiBaseUrl = apiUrl;
  }

  async getGames(): Promise<any> {
    try {
      const response = await fetch(`${this.apiBaseUrl}/api/games`);
      
      if (!response.ok) {
        throw new Error(`HTTP error! status: ${response.status}`);
      }

      const data = await response.json();
      console.log('Games data:', data);
      return data;
    } catch (error) {
      console.error('Error getting games:', error);
      throw error;
    }
  }

  async getGamesList(): Promise<any> {
    try {
      const response = await fetch(`${this.apiBaseUrl}/api/games_list`);
      
      if (!response.ok) {
        throw new Error(`HTTP error! status: ${response.status}`);
      }

      const data = await response.json();
      console.log('Games list:', data);
      return data;
    } catch (error) {
      console.error('Error getting games list:', error);
      throw error;
    }
  }

  async getServers(): Promise<any> {
    try {
      const response = await fetch(`${this.apiBaseUrl}/api/servers`);
      
      if (!response.ok) {
        throw new Error(`HTTP error! status: ${response.status}`);
      }

      const data = await response.json();
      console.log('Servers:', data);
      return data;
    } catch (error) {
      console.error('Error getting servers:', error);
      throw error;
    }
  }

  async getRares(): Promise<any> {
    try {
      const response = await fetch(`${this.apiBaseUrl}/api/rares`);
      
      if (!response.ok) {
        throw new Error(`HTTP error! status: ${response.status}`);
      }

      const data = await response.json();
      console.log('Rares:', data);
      return data;
    } catch (error) {
      console.error('Error getting rares:', error);
      throw error;
    }
  }

  /**
   * 获取账号列表
   */
  async getAccounts(params?: {
    game_name?: string;
    target_min?: number;
    target_max?: number;
    server_area?: string;
  }): Promise<any> {
    try {
      let url = `${this.apiBaseUrl}/api/accounts`;
      if (params) {
        const queryParams = new URLSearchParams();
        if (params.game_name) queryParams.append('game_name', params.game_name);
        if (params.target_min !== undefined) queryParams.append('target_min', params.target_min.toString());
        if (params.target_max !== undefined) queryParams.append('target_max', params.target_max.toString());
        if (params.server_area) queryParams.append('server_area', params.server_area);
        url += '?' + queryParams.toString();
      }
      
      const response = await fetch(url);
      
      if (!response.ok) {
        throw new Error(`HTTP error! status: ${response.status}`);
      }

      const data = await response.json();
      console.log('Accounts:', data);
      return data;
    } catch (error) {
      console.error('Error getting accounts:', error);
      throw error;
    }
  }

  /**
   * 获取账号详情
   */
  async getAccountDetail(accountId: string): Promise<any> {
    try {
      const response = await fetch(`${this.apiBaseUrl}/api/accounts/${accountId}`);
      
      if (!response.ok) {
        throw new Error(`HTTP error! status: ${response.status}`);
      }

      const data = await response.json();
      console.log('Account detail:', data);
      return data;
    } catch (error) {
      console.error('Error getting account detail:', error);
      throw error;
    }
  }

  /**
   * 购买账号
   */
  async buyAccount(accountId: string): Promise<any> {
    try {
      const response = await fetch(`${this.apiBaseUrl}/api/accounts/${accountId}/buy`, {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
        },
        body: JSON.stringify({ account_id: accountId }),
      });
      
      if (!response.ok) {
        throw new Error(`HTTP error! status: ${response.status}`);
      }

      const data = await response.json();
      console.log('Buy account result:', data);
      return data;
    } catch (error) {
      console.error('Error buying account:', error);
      throw error;
    }
  }

  /**
   * 健康检查
   */
  async healthCheck(): Promise<boolean> {
    try {
      const response = await fetch(`${this.baseUrl}/ok`);
      return response.ok;
    } catch (error) {
      console.error('Health check failed:', error);
      return false;
    }
  }

  /**
   * 发送消息到后端并获取响应
   */
  async sendMessage(content: string): Promise<any> {
    try {
      console.log('Sending message to backend:', content);

      // 使用 FastAPI 的 /api/chat 接口
      const response = await fetch(`${this.apiBaseUrl}/api/chat`, {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
        },
        body: JSON.stringify({
          message: content,
          conversation_history: [],
        }),
      });

      console.log('Response status:', response.status);

      if (!response.ok) {
        const errorText = await response.text();
        console.error('Error response:', errorText);
        throw new Error(`HTTP error! status: ${response.status}`);
      }

      const data = await response.json();
      console.log('Response data:', data);
      return data;
    } catch (error) {
      console.error('Error sending message:', error);
      throw error;
    }
  }

  /**
   * 获取运行状态和结果
   */
  async getRunStatus(runId: string): Promise<any> {
    try {
      const response = await fetch(`${this.baseUrl}/runs/${runId}`, {
        method: 'GET',
      });

      if (!response.ok) {
        throw new Error(`HTTP error! status: ${response.status}`);
      }

      return await response.json();
    } catch (error) {
      console.error('Error getting run status:', error);
      throw error;
    }
  }

  /**
   * 轮询直到运行完成
   */
  async waitForRun(runId: string, maxRetries: number = 30, retryDelay: number = 1000): Promise<any> {
    for (let i = 0; i < maxRetries; i++) {
      const status = await this.getRunStatus(runId);
      
      if (status.status === 'success') {
        return status;
      }
      
      if (status.status === 'error') {
        throw new Error('Run failed');
      }

      await new Promise(resolve => setTimeout(resolve, retryDelay));
    }

    throw new Error('Run timeout');
  }
}

// 创建全局 API 客户端实例
export const apiClient = new ApiClient(import.meta.env.VITE_BACKEND_URL, import.meta.env.VITE_API_URL);
