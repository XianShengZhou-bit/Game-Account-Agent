export interface Game {
  id: number;
  name: string;
  icon: string;
  image?: string;
}

export interface Account {
  id: string;
  game_name: string;
  server_area: string;
  title: string;
  price: number;
  account_level: number;
  hero_count: number;
  skin_count: number;
  rare_items: string;
  description: string;
  status: number;
  version: number;
  created_at: string;
}

export interface Message {
  id: string;
  type: 'human' | 'ai' | 'interrupt' | 'order_completed' | 'error';
  content: string;
  required_fields?: string[];
  order_info?: {
    order_sn: string;
    account_id: number;
    buyer_phone: string;
    amount: number;
  };
  timestamp: Date;
}
